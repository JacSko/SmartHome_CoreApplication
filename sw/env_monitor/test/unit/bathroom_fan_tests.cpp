#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/bathroom_fan.c"
#ifdef __cplusplus
}
#endif

#include "task_scheduler_mock.h"
#include "logger_mock.h"
#include "relays_board_mock.h"
#include "env_monitor_mock.h"

/* ============================= */
/**
 * @file bathroom_fan_tests.cpp
 *
 * @brief Unit tests of FAN module
 *
 * @details
 * This tests verifies behavior of FAN module
 *
 * @author Jacek Skowronek
 * @date 28/12/2020
 */
/* ============================= */


using namespace ::testing;

struct callbackMock
{
   MOCK_METHOD1(callback, void(FAN_STATE));
};

callbackMock* callMock;

void fake_callback(FAN_STATE state)
{
   callMock->callback(state);
}

struct fanFixture : public ::testing::Test
{
   virtual void SetUp()
   {
      mock_sch_init();
      mock_logger_init();
      mock_rel_init();
      mock_env_init();
      callMock = new callbackMock;
      FAN_CONFIG config;
      config.fan_humidity_threshold = DEFAULT_HUM_THR;
      config.fan_threshold_hysteresis = DEFAULT_HYSTER;
      config.max_working_time_s = DEFAULT_MAX_TIME;
      config.min_working_time_s = DEFAULT_MIN_TIME;

      EXPECT_CALL(*rel_mock, rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_OFF));
      EXPECT_CALL(*env_mock, env_register_listener(_, ENV_BATHROOM)).WillOnce(Return(RETURN_OK));
      EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, _, 1000, TASKSTATE_STOPPED, TASKTYPE_PERIODIC)).WillOnce(Return(RETURN_OK));
      EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).WillOnce(Return(RETURN_OK));
      EXPECT_EQ(RETURN_OK, fan_initialize(&config));

   }

   virtual void TearDown()
   {
      EXPECT_CALL(*env_mock, env_unregister_listener(_,_));
      EXPECT_CALL(*sch_mock, sch_unsubscribe(_));
      fan_deinitialize();

      mock_sch_deinit();
      mock_logger_deinit();
      mock_rel_deinit();
      mock_env_deinit();
      delete callMock;
   }

   void SIMULATE_TIME(uint8_t seconds)
   {
      for (uint8_t i = 0; i < seconds; i++)
      {
         fan_on_timeout();
      }
   }

   uint8_t DEFAULT_HUM_THR = 80;
   uint8_t DEFAULT_HYSTER = 5;
   uint8_t DEFAULT_MAX_TIME = 100;
   uint8_t DEFAULT_MIN_TIME = 50;
};

/**
 * @test Initialization of FAN module
 */
TEST(fanInitialization, fan_initialization_tests)
{
   mock_sch_init();
   mock_logger_init();
   mock_rel_init();
   mock_env_init();

   FAN_CONFIG config;
   config.fan_humidity_threshold = 80U;
   config.fan_threshold_hysteresis = 5;
   config.max_working_time_s = 100;
   config.min_working_time_s = 50;

   /**
    * <b>scenario</b>: No config provided.<br>
    * <b>expected</b>: Module not initialized.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, fan_initialize(nullptr));

   /**
    * <b>scenario</b>: Cannot register to env module.<br>
    * <b>expected</b>: Module not initialized.<br>
    * ************************************************
    */
   EXPECT_CALL(*env_mock, env_register_listener(_, ENV_BATHROOM)).WillOnce(Return(RETURN_NOK));
   EXPECT_EQ(RETURN_NOK, fan_initialize(&config));

   /**
    * <b>scenario</b>: Module initialization.<br>
    * <b>expected</b>: Module initialized.<br>
    * ************************************************
    */
   EXPECT_CALL(*rel_mock, rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_OFF));
   EXPECT_CALL(*env_mock, env_register_listener(_, ENV_BATHROOM)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, _, 1000, TASKSTATE_STOPPED, TASKTYPE_PERIODIC)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_OK, fan_initialize(&config));


   mock_sch_deinit();
   mock_logger_deinit();
   mock_rel_deinit();
   mock_env_deinit();

}

/**
 * @test Fan started, stopped by humidity drop
 */
TEST_F(fanFixture, fan_start_stop_hum_drop)
{
   ASSERT_EQ(fan_get_state(), FAN_STATE_OFF);
   DHT_SENSOR sensor;
   sensor.type = DHT_TYPE_DHT11;
   sensor.data.hum_h = DEFAULT_HUM_THR + 1;

   EXPECT_EQ(RETURN_OK, fan_add_listener(&fake_callback));
   /**
    * <b>scenario</b>: Error event received.<br>
    * <b>expected</b>: Fan not started.<br>
    * ************************************************
    */
   fan_on_env_data(ENV_EV_ERROR, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_OFF);
   /**
    * <b>scenario</b>: ID different than BATHROOM received.<br>
    * <b>expected</b>: Fan not started.<br>
    * ************************************************
    */
   fan_on_env_data(ENV_EV_NEW_DATA, ENV_BEDROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_OFF);

   /**
    * <b>scenario</b>: Too low humidity.<br>
    * <b>expected</b>: Fan not started.<br>
    * ************************************************
    */
   sensor.data.hum_h = DEFAULT_HUM_THR - 1;
   fan_on_env_data(ENV_EV_NEW_DATA, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_OFF);

   /**
    * <b>scenario</b>: Humidity above threshold, cannot start scheduler task.<br>
    * <b>expected</b>: Fan not started.<br>
    * ************************************************
    */
   sensor.data.hum_h = DEFAULT_HUM_THR + 1;

   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_RUNNING)).WillOnce(Return(RETURN_NOK));
   fan_on_env_data(ENV_EV_NEW_DATA, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_OFF);

   /**
    * <b>scenario</b>: Humidity above threshold, cannot set relay state.<br>
    * <b>expected</b>: Fan not started.<br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_RUNNING)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*rel_mock, rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_ON)).WillOnce(Return(RETURN_NOK));
   fan_on_env_data(ENV_EV_NEW_DATA, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_OFF);

   /**
    * <b>scenario</b>: Humidity above threshold.<br>
    * <b>expected</b>: Fan started.<br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_RUNNING)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*rel_mock, rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_ON)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*callMock, callback(FAN_STATE_ON));
   fan_on_env_data(ENV_EV_NEW_DATA, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_ON);

   /**
    * <b>scenario</b>: Received error from ENV when fan is running.<br>
    * <b>expected</b>: Fan not stopped.<br>
    * ************************************************
    */
   fan_on_env_data(ENV_EV_ERROR, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_ON);

   /**
    * <b>scenario</b>: Humidity drop to lower limit.<br>
    * <b>expected</b>: Fan not stopped.<br>
    * ************************************************
    */
   sensor.data.hum_h = DEFAULT_HUM_THR - DEFAULT_HYSTER + 1;
   fan_on_env_data(ENV_EV_NEW_DATA, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_ON);

   SIMULATE_TIME(30);

   /**
    * <b>scenario</b>: Humidity dropped below limit but min working time not met.<br>
    * <b>expected</b>: Fan stopped.<br>
    * ************************************************
    */
   sensor.data.hum_h = DEFAULT_HUM_THR - DEFAULT_HYSTER;
   fan_on_env_data(ENV_EV_NEW_DATA, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_ON);

   SIMULATE_TIME(30);
   /**
    * <b>scenario</b>: Humidity dropped below limit.<br>
    * <b>expected</b>: Fan stopped.<br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*rel_mock, rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_OFF)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*callMock, callback(FAN_STATE_OFF));
   fan_on_env_data(ENV_EV_NEW_DATA, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_OFF);

   EXPECT_EQ(RETURN_OK, fan_remove_listener(&fake_callback));
}

/**
 * @test Fan started, stopped by humidity drop
 */
TEST_F(fanFixture, fan_start_stop_timeout)
{
   ASSERT_EQ(fan_get_state(), FAN_STATE_OFF);
   DHT_SENSOR sensor;
   sensor.type = DHT_TYPE_DHT11;
   sensor.data.hum_h = DEFAULT_HUM_THR + 1;

   EXPECT_EQ(RETURN_OK, fan_add_listener(&fake_callback));

   /**
    * <b>scenario</b>: Fan running, timeout occurs.<br>
    * <b>expected</b>: Fan switched to suspend mode.<br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_RUNNING)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*rel_mock, rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_ON)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*callMock, callback(FAN_STATE_ON));
   fan_on_env_data(ENV_EV_NEW_DATA, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_ON);
   SIMULATE_TIME(99);

   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*rel_mock, rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_OFF)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*callMock, callback(FAN_STATE_SUSPEND));

   fan_on_timeout();
   EXPECT_EQ(fan_get_state(), FAN_STATE_SUSPEND);

   /**
    * <b>scenario</b>: Humidity drop below threshold.<br>
    * <b>expected</b>: Fan switched to OFF mode.<br>
    * ************************************************
    */
   sensor.data.hum_h = DEFAULT_HUM_THR - DEFAULT_HYSTER;
   EXPECT_CALL(*callMock, callback(FAN_STATE_OFF));
   fan_on_env_data(ENV_EV_NEW_DATA, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_OFF);

   /**
    * <b>scenario</b>: Humidity raised above threshold.<br>
    * <b>expected</b>: Fan started.<br>
    * ************************************************
    */
   sensor.data.hum_h = DEFAULT_HUM_THR + 1;
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_RUNNING)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*rel_mock, rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_ON)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*callMock, callback(FAN_STATE_ON));
   fan_on_env_data(ENV_EV_NEW_DATA, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_ON);

   EXPECT_EQ(RETURN_OK, fan_remove_listener(&fake_callback));

}

/**
 * @test Manual fan starting/stopping
 */
TEST_F(fanFixture, fan_start_stop_manual)
{
   ASSERT_EQ(fan_get_state(), FAN_STATE_OFF);
   DHT_SENSOR sensor;
   sensor.type = DHT_TYPE_DHT11;
   sensor.data.hum_h = DEFAULT_HUM_THR + 1;

   EXPECT_EQ(RETURN_OK, fan_add_listener(&fake_callback));

   /**
    * <b>scenario</b>: Fan started manually, cannot set relay.<br>
    * <b>expected</b>: Fan not started.<br>
    * ************************************************
    */
   EXPECT_CALL(*rel_mock, rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_ON)).WillOnce(Return(RETURN_NOK));
   EXPECT_EQ(RETURN_NOK, fan_start());
   EXPECT_EQ(FAN_STATE_OFF, fan_get_state());

   /**
    * <b>scenario</b>: Fan started manually.<br>
    * <b>expected</b>: Fan started.<br>
    * ************************************************
    */
   EXPECT_CALL(*rel_mock, rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_ON)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*callMock, callback(FAN_STATE_ON));
   EXPECT_EQ(RETURN_OK, fan_start());
   EXPECT_EQ(FAN_STATE_ON, fan_get_state());

   /**
    * <b>scenario</b>: Low humidity received.<br>
    * <b>expected</b>: Fan not stopped.<br>
    * ************************************************
    */
   sensor.data.hum_h = DEFAULT_HUM_THR - DEFAULT_HYSTER;
   fan_on_env_data(ENV_EV_NEW_DATA, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_ON);

   /**
    * <b>scenario</b>: High humidity received.<br>
    * <b>expected</b>: Fan state not changed.<br>
    * ************************************************
    */
   sensor.data.hum_h = DEFAULT_HUM_THR + 1;
   fan_on_env_data(ENV_EV_NEW_DATA, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_ON);

   /**
    * <b>scenario</b>: Fan running, start requested again.<br>
    * <b>expected</b>: RETURN_NOK returned.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, fan_start());

   /**
    * <b>scenario</b>: Fan stoped manually.<br>
    * <b>expected</b>: Fan stopped.<br>
    * ************************************************
    */
   EXPECT_CALL(*rel_mock, rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_OFF)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*callMock, callback(FAN_STATE_OFF));
   EXPECT_EQ(RETURN_OK, fan_stop());
   EXPECT_EQ(FAN_STATE_OFF, fan_get_state());
   /**
    * <b>scenario</b>: Fan stopped, stop requested again.<br>
    * <b>expected</b>: RETURN_NOK returned.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, fan_stop());

   /**
    * <b>scenario</b>: Humidity raised above threshold.<br>
    * <b>expected</b>: Fan started.<br>
    * ************************************************
    */
   sensor.data.hum_h = DEFAULT_HUM_THR + 1;
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_RUNNING)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*rel_mock, rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_ON)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*callMock, callback(FAN_STATE_ON));
   fan_on_env_data(ENV_EV_NEW_DATA, ENV_BATHROOM, &sensor);
   EXPECT_EQ(fan_get_state(), FAN_STATE_ON);

   EXPECT_EQ(RETURN_OK, fan_remove_listener(&fake_callback));

}

/**
 * @test Setting FAN working times tests
 */
TEST_F(fanFixture, fan_setting_working_times_tests)
{
   uint16_t CORRECT_MIN_TIME = 50;
   uint16_t CORRECT_MAX_TIME = 7200;
   /**
    * <b>scenario</b>: Working times are out of limit.<br>
    * <b>expected</b>: Working times not changed.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, fan_set_min_working_time(FAN_MIN_WORKING_TIME_S - 1));
   EXPECT_EQ(RETURN_NOK, fan_set_max_working_time(FAN_MAX_WORKING_TIME_S + 1));

   /**
    * <b>scenario</b>: Working times are in limit.<br>
    * <b>expected</b>: Working times not changed.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_OK, fan_set_min_working_time(CORRECT_MIN_TIME));
   EXPECT_EQ(RETURN_OK, fan_set_max_working_time(CORRECT_MAX_TIME));

   FAN_CONFIG config;
   EXPECT_EQ(RETURN_NOK, fan_get_config(nullptr));
   EXPECT_EQ(RETURN_OK, fan_get_config(&config));

   EXPECT_EQ(config.max_working_time_s, CORRECT_MAX_TIME);
   EXPECT_EQ(config.min_working_time_s, CORRECT_MIN_TIME);

}

/**
 * @test Setting FAN humidity threshold tests
 */
TEST_F(fanFixture, fan_setting_threshold_tests)
{
   uint8_t INVALID_HYSTERESIS = 100;
   uint8_t CORRECT_HYSTERESIS = 10;
   uint8_t CORRECT_THRESHOLD = 70;

   /**
    * <b>scenario</b>: Humidity thresholds out of limit.<br>
    * <b>expected</b>: Thresholds not changed.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, fan_set_humidity_threshold(FAN_MIN_HUMIDITY_THR - 1));
   EXPECT_EQ(RETURN_NOK, fan_set_humidity_threshold(FAN_MAX_HUMIDITY_THR + 1));
   EXPECT_EQ(RETURN_NOK, fan_set_threshold_hysteresis(INVALID_HYSTERESIS));

   /**
    * <b>scenario</b>: Humidity thresholds in limits.<br>
    * <b>expected</b>: Thresholds changed.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_OK, fan_set_humidity_threshold(CORRECT_THRESHOLD));
   EXPECT_EQ(RETURN_OK, fan_set_threshold_hysteresis(CORRECT_HYSTERESIS));

   FAN_CONFIG config;
   EXPECT_EQ(RETURN_OK, fan_get_config(&config));

   EXPECT_EQ(config.fan_humidity_threshold, CORRECT_THRESHOLD);
   EXPECT_EQ(config.fan_threshold_hysteresis, CORRECT_HYSTERESIS);

}
