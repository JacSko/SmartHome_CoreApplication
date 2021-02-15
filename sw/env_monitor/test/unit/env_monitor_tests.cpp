#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/env_monitor.c"
#ifdef __cplusplus
}
#endif

#include "task_scheduler_mock.h"
#include "logger_mock.h"
#include "dht_driver_mock.h"

/* ============================= */
/**
 * @file env_monitor_tests.cpp
 *
 * @brief Unit tests of ENV Monitor module
 *
 * @details
 * This tests verifies behavior of ENV module
 *
 * @author Jacek Skowronek
 * @date 27/12/2020
 */
/* ============================= */


using namespace ::testing;

struct callbackMock
{
   MOCK_METHOD3(callback, void(ENV_EVENT, ENV_ITEM_ID,  const DHT_SENSOR*));
};

callbackMock* callMock;

void fake_callback(ENV_EVENT event, ENV_ITEM_ID id,  const DHT_SENSOR* sensor)
{
   callMock->callback(event, id, sensor);
}

struct envFixture : public ::testing::Test
{
   virtual void SetUp()
   {
      cfg.measure_running = 0x01;
      cfg.max_nr_rate = 99;
      cfg.max_cs_rate = 99;
      cfg.items[0].dht_id = DHT_SENSOR1;
      cfg.items[0].env_id = ENV_OUTSIDE;
      cfg.items[1].dht_id = DHT_SENSOR2;
      cfg.items[1].env_id = ENV_WARDROBE;
      cfg.items[2].dht_id = DHT_SENSOR3;
      cfg.items[2].env_id = ENV_BEDROOM;
      cfg.items[3].dht_id = DHT_SENSOR4;
      cfg.items[3].env_id = ENV_BATHROOM;
      cfg.items[4].dht_id = DHT_SENSOR5;
      cfg.items[4].env_id = ENV_KITCHEN;
      cfg.items[5].dht_id = DHT_SENSOR6;
      cfg.items[5].env_id = ENV_STAIRS;

      mock_sch_init();
      mock_logger_init();
      mock_dht_init();

      EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, TASKPRIO_LOW, ENV_MEASURE_PERIOD_DEF_MS, TASKSTATE_RUNNING, TASKTYPE_PERIODIC)).WillOnce(Return(RETURN_OK));
      EXPECT_EQ(RETURN_OK, env_initialize(&cfg));
      callMock = new callbackMock;
   }

   virtual void TearDown()
   {
      mock_sch_deinit();
      mock_logger_deinit();
      mock_dht_deinit();
      env_deinitialize();
      delete callMock;
   }

   ENV_CONFIG cfg;
};

/**
 * @test
 */
TEST(envInitialization, env_initialization_tests)
{
   ENV_CONFIG cfg;
   cfg.measure_running = 0x01;
   cfg.max_nr_rate = 99;
   cfg.max_cs_rate = 99;
   cfg.items[0].dht_id = DHT_SENSOR1;
   cfg.items[0].env_id = ENV_OUTSIDE;
   cfg.items[1].dht_id = DHT_SENSOR2;
   cfg.items[1].env_id = ENV_WARDROBE;
   cfg.items[2].dht_id = DHT_SENSOR3;
   cfg.items[2].env_id = ENV_BEDROOM;
   cfg.items[3].dht_id = DHT_SENSOR4;
   cfg.items[3].env_id = ENV_BATHROOM;
   cfg.items[4].dht_id = DHT_SENSOR5;
   cfg.items[4].env_id = ENV_KITCHEN;
   cfg.items[5].dht_id = DHT_SENSOR6;
   cfg.items[5].env_id = ENV_STAIRS;

   mock_sch_init();
   mock_logger_init();
   mock_dht_init();

   /**
    * <b>scenario</b>: ENV initialize - no config provided.<br>
    * <b>expected</b>: Module not intialized.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, env_initialize(nullptr));

   /**
    * <b>scenario</b>: ENV initialize - cannot subscribe scheduler task.<br>
    * <b>expected</b>: Module not intialized.<br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, _, _, _, _)).WillOnce(Return(RETURN_NOK));
   EXPECT_EQ(RETURN_NOK, env_initialize(&cfg));

   /**
    * <b>scenario</b>: ENV initialize.<br>
    * <b>expected</b>: Module intialized.<br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, _, _, _, _)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_OK, env_initialize(&cfg));

   /**
    * <b>scenario</b>: Get config when no buffer provided.<br>
    * <b>expected</b>: RETURN_NOK returned.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, env_get_config(nullptr));

   /**
    * <b>scenario</b>: Get ENV config.<br>
    * <b>expected</b>: RETURN_OK returned.<br>
    * ************************************************
    */
   ENV_CONFIG result = {};
   EXPECT_EQ(RETURN_OK, env_get_config(&result));

   EXPECT_EQ(result.measure_running, 0x01);
   EXPECT_EQ(result.max_cs_rate, 99);
   EXPECT_EQ(result.max_nr_rate, 99);


   mock_sch_deinit();
   mock_logger_deinit();
   mock_dht_deinit();
   env_deinitialize();


}

/**
 * @test Reading sensors sequence test
 */
TEST_F(envFixture, env_reading_test)
{

   EXPECT_EQ(RETURN_OK, env_register_listener(&fake_callback, ENV_OUTSIDE));
   EXPECT_EQ(RETURN_OK, env_register_listener(&fake_callback, ENV_WARDROBE));
   EXPECT_EQ(RETURN_OK, env_register_listener(&fake_callback, ENV_BEDROOM));
   EXPECT_EQ(RETURN_OK, env_register_listener(&fake_callback, ENV_BATHROOM));
   EXPECT_EQ(RETURN_OK, env_register_listener(&fake_callback, ENV_KITCHEN));
   EXPECT_EQ(RETURN_OK, env_register_listener(&fake_callback, ENV_STAIRS));

   DHT_SENSOR sensor = {};
   sensor.id = DHT_SENSOR1;
   sensor.type = DHT_TYPE_DHT11;
   sensor.data.temp_h = 30;
   sensor.data.temp_l = 2;
   sensor.data.hum_h = 60;
   sensor.data.hum_l = 1;

   /**
    * <b>scenario</b>: Simulate timeout - reading one sensor.<br>
    * <b>expected</b>: Data received, callback called, next sensor selected.<br>
    * ************************************************
    */

   /* ENV_OUTSIDE should be selected first - so reading DHT_SENSOR1 */
   EXPECT_CALL(*dht_mock, dht_read_async(DHT_SENSOR1, _)).WillOnce(Return(RETURN_OK));
   env_on_timeout();
   EXPECT_CALL(*callMock, callback(ENV_EV_NEW_DATA, ENV_OUTSIDE,_));
   env_on_dht_data(DHT_STATUS_OK, &sensor);

   /* ENV_WARDROBE should be selected next - so reading DHT_SENSOR2 */
   EXPECT_CALL(*dht_mock, dht_read_async(DHT_SENSOR2, _)).WillOnce(Return(RETURN_OK));
   env_on_timeout();
   EXPECT_CALL(*callMock, callback(ENV_EV_NEW_DATA, ENV_WARDROBE,_));
   env_on_dht_data(DHT_STATUS_OK, &sensor);

   /* ENV_BEDROOM should be selected next - so reading DHT_SENSOR3 */
   EXPECT_CALL(*dht_mock, dht_read_async(DHT_SENSOR3, _)).WillOnce(Return(RETURN_OK));
   env_on_timeout();
   EXPECT_CALL(*callMock, callback(ENV_EV_NEW_DATA, ENV_BEDROOM,_));
   env_on_dht_data(DHT_STATUS_OK, &sensor);

   /* ENV_BATHROOM should be selected next - so reading DHT_SENSOR4 */
   EXPECT_CALL(*dht_mock, dht_read_async(DHT_SENSOR4, _)).WillOnce(Return(RETURN_OK));
   env_on_timeout();
   EXPECT_CALL(*callMock, callback(ENV_EV_NEW_DATA, ENV_BATHROOM,_));
   env_on_dht_data(DHT_STATUS_OK, &sensor);

   /* ENV_KITCHEN should be selected next - so reading DHT_SENSOR5 */
   EXPECT_CALL(*dht_mock, dht_read_async(DHT_SENSOR5, _)).WillOnce(Return(RETURN_OK));
   env_on_timeout();
   EXPECT_CALL(*callMock, callback(ENV_EV_NEW_DATA, ENV_KITCHEN,_));
   env_on_dht_data(DHT_STATUS_OK, &sensor);

   /* ENV_WARDROBE should be selected next - so reading DHT_SENSOR6 */
   EXPECT_CALL(*dht_mock, dht_read_async(DHT_SENSOR6, _)).WillOnce(Return(RETURN_OK));
   env_on_timeout();
   EXPECT_CALL(*callMock, callback(ENV_EV_NEW_DATA, ENV_STAIRS,_));
   env_on_dht_data(DHT_STATUS_OK, &sensor);

   /* ENV_OUTSIDE should be selected again - so reading DHT_SENSOR1 */
   EXPECT_CALL(*dht_mock, dht_read_async(DHT_SENSOR1, _)).WillOnce(Return(RETURN_OK));
   env_on_timeout();
   EXPECT_CALL(*callMock, callback(ENV_EV_NEW_DATA, ENV_OUTSIDE,_));
   env_on_dht_data(DHT_STATUS_OK, &sensor);


}


/**
 * @test Adding and removing module listeners
 */
TEST_F(envFixture, env_listener_add_remove)
{

   /**
    * <b>scenario</b>: Add the same listener for all IDs.<br>
    * <b>expected</b>: RETURN_OK returned for all IDs.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_OK, env_register_listener(&fake_callback, ENV_OUTSIDE));
   EXPECT_EQ(RETURN_OK, env_register_listener(&fake_callback, ENV_WARDROBE));
   EXPECT_EQ(RETURN_OK, env_register_listener(&fake_callback, ENV_BEDROOM));
   EXPECT_EQ(RETURN_OK, env_register_listener(&fake_callback, ENV_BATHROOM));
   EXPECT_EQ(RETURN_OK, env_register_listener(&fake_callback, ENV_KITCHEN));
   EXPECT_EQ(RETURN_OK, env_register_listener(&fake_callback, ENV_STAIRS));

   /**
    * <b>scenario</b>: Add the same listener second time.<br>
    * <b>expected</b>: RETURN_ERROR returned.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_ERROR, env_register_listener(&fake_callback, ENV_OUTSIDE));

   /**
    * <b>scenario</b>: Remove listener, than add it again.<br>
    * <b>expected</b>: RETURN_OK returned.<br>
    * ************************************************
    */
   env_unregister_listener(&fake_callback, ENV_OUTSIDE);
   EXPECT_EQ(RETURN_OK, env_register_listener(&fake_callback, ENV_OUTSIDE));

}

/**
 * @test Setting measurement period
 */
TEST_F(envFixture, env_setting_measurement_period)
{

   /**
    * <b>scenario</b>: Set measurement period below limit.<br>
    * <b>expected</b>: Period not changed.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, env_set_measurement_period(ENV_MEASURE_PERIOD_MIN_MS - 1));
   /**
    * <b>scenario</b>: Set measurement period above limit.<br>
    * <b>expected</b>: Period not changed.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, env_set_measurement_period(ENV_MEASURE_PERIOD_MAX_MS + 1));

   /**
    * <b>scenario</b>: Scheduler do not accept new period.<br>
    * <b>expected</b>: Period not changed.<br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_period(_,_)).WillOnce(Return(RETURN_NOK));
   EXPECT_EQ(RETURN_NOK, env_set_measurement_period(ENV_MEASURE_PERIOD_DEF_MS + 1));

   /**
    * <b>scenario</b>: Set correct measurement period.<br>
    * <b>expected</b>: Period not changed.<br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_period(_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_OK, env_set_measurement_period(ENV_MEASURE_PERIOD_DEF_MS + 1));
   EXPECT_EQ(ENV_MEASURE_PERIOD_DEF_MS + 1, env_get_measurement_period());

}

/**
 * @test Reading one sensor standalone
 */
TEST_F(envFixture, env_reading_one_sensor)
{

   DHT_SENSOR sensor = {};

   /**
    * <b>scenario</b>: No buffer provided.<br>
    * <b>expected</b>: Sensor not read.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_ERROR, env_read_sensor(ENV_OUTSIDE, NULL));

   /**
    * <b>scenario</b>: No response from sensor.<br>
    * <b>expected</b>: RETURN_NOK returned.<br>
    * ************************************************
    */
   EXPECT_CALL(*dht_mock, dht_read(DHT_SENSOR1, _)).WillOnce(Return(DHT_STATUS_NO_RESPONSE));
   EXPECT_EQ(RETURN_NOK, env_read_sensor(ENV_OUTSIDE, &sensor));

   /**
    * <b>scenario</b>: Data read correctly.<br>
    * <b>expected</b>: RETURN_OK returned.<br>
    * ************************************************
    */
   EXPECT_CALL(*dht_mock, dht_read(DHT_SENSOR1, _)).WillOnce(Invoke([&]
                                    (DHT_SENSOR_ID id, DHT_SENSOR *sensor) -> DHT_STATUS
                                    {
                                       sensor->data.temp_h = 30;
                                       sensor->data.hum_h = 60;
                                       return DHT_STATUS_OK;
                                    }));
   EXPECT_EQ(RETURN_OK, env_read_sensor(ENV_OUTSIDE, &sensor));
   EXPECT_EQ(sensor.data.temp_h, 30);
   EXPECT_EQ(sensor.data.hum_h, 60);

}

/**
 * @test Error rate calculation tests
 */
TEST_F(envFixture, env_error_rates_calc_tests)
{
   ENV_ITEM_ID ID = ENV_BEDROOM;
   DHT_SENSOR sensor = {};
   /**
    * <b>scenario</b>: Initial CS_RATE value check.<br>
    * <b>expected</b>: 0 should be returned.<br>
    * ************************************************
    */
   ENV_ERROR_RATE result = env_get_error_stats(ID);
   EXPECT_EQ(result.cs_err_rate, 0);

   /**
    * <b>scenario</b>: 10 measurements with CS error added.<br>
    * <b>expected</b>: CS_RATE has correct value in percents.<br>
    * ************************************************
    */
   for (uint8_t i = 0; i < 10; i++)
   {
      env_module.selected_sensor = &env_module.sensors[2]; /* manually selected sensor ENV_BEDROOM */
      env_on_dht_data(DHT_STATUS_CHECKSUM_ERROR, &sensor);
   }
   result = env_get_error_stats(ID);
   EXPECT_EQ(result.cs_err_rate, 10);

   /**
    * <b>scenario</b>: 95 correct measurements added.<br>
    * <b>expected</b>: CS_RATE has correct value in percents.<br>
    * ************************************************
    */
   for (uint8_t i = 0; i < 91; i++)
   {
      env_module.selected_sensor = &env_module.sensors[2]; /* manually selected sensor ENV_BEDROOM */
      env_on_dht_data(DHT_STATUS_OK, &sensor);
   }
   result = env_get_error_stats(ID);
   EXPECT_EQ(result.cs_err_rate, 9);

   /**
    * <b>scenario</b>: 2 no response error added.<br>
    * <b>expected</b>: CS_RATE and NR_RATE has correct values in percents.<br>
    * ************************************************
    */
   for (uint8_t i = 0; i < 2; i++)
   {
      env_module.selected_sensor = &env_module.sensors[2]; /* manually selected sensor ENV_BEDROOM */
      env_on_dht_data(DHT_STATUS_NO_RESPONSE, &sensor);
   }
   result = env_get_error_stats(ID);
   EXPECT_EQ(result.cs_err_rate, 7);
   EXPECT_EQ(result.nr_err_rate, 2);
}

/**
 * @test Getting data from defined sensor
 */
TEST_F(envFixture, env_getting_sensor_datat)
{

   DHT_SENSOR sensor = {};

   /**
    * <b>scenario</b>: No buffer provided.<br>
    * <b>expected</b>: RETURN_ERROR returned.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_ERROR, env_get_sensor_data(ENV_OUTSIDE, NULL));

   /**
    * <b>scenario</b>: Requested sensor not found.<br>
    * <b>expected</b>: RETURN_NOK returned.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_ERROR, env_get_sensor_data(ENV_UNKNOWN_ITEM, &sensor));

   /**
    * <b>scenario</b>: Data gt correctly.<br>
    * <b>expected</b>: RETURN_OK returned.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_OK, env_get_sensor_data(ENV_OUTSIDE, &sensor));
}


