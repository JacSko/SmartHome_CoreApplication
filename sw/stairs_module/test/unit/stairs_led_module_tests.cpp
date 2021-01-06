#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/stairs_led_module.c"
#ifdef __cplusplus
}
#endif

#include "i2c_driver_mock.h"
#include "inputs_board_mock.h"
#include "task_scheduler_mock.h"
#include "logger_mock.h"

/* ============================= */
/**
 * @file stairs_led_module_tests.cpp
 *
 * @brief Unit tests of Stairs LED module
 *
 * @details
 * This tests verifies behavior of Stairs LED module
 *
 * @author Jacek Skowronek
 * @date 31/12/2020
 */
/* ============================= */


using namespace ::testing;

MATCHER_P(I2C_DATA_MATCH, byte1, "I2C_DATA_MATCH error")
{
   if (!arg)
   {
      return false;
   }
   return byte1 == arg[0];
}

struct ledFixture : public ::testing::Test
{
	virtual void SetUp()
	{
	   mock_logger_init();
	   mock_sch_init();
	   mock_i2c_init();
	   mock_inp_init();
	   SLM_CONFIG config = {};
	   config.address = 0x11;
	   config.off_effect_mode = SLM_OFF_EFFECT_ENABLED;
	   config.program_id = SLM_PROGRAM1;
	   EXPECT_CALL(*i2c_mock, i2c_write(_, _, _)).WillOnce(Return(I2C_STATUS_OK));
	   EXPECT_CALL(*sch_mock, sch_set_task_period(_, _));
	   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
	   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, TASKPRIO_LOW, _, TASKSTATE_STOPPED, TASKTYPE_TRIGGER)).WillOnce(Return(RETURN_OK));
	   EXPECT_CALL(*inp_mock, inp_add_input_listener(_)).WillOnce(Return(RETURN_OK));
	   EXPECT_EQ(RETURN_OK, slm_initialize(&config));

	}

	virtual void TearDown()
	{
	   mock_logger_deinit();
	   mock_sch_deinit();
	   mock_i2c_deinit();
	   mock_inp_deinit();
	}
};

struct ledFixtureNoEffect : public ::testing::Test
{
   virtual void SetUp()
   {
      mock_logger_init();
      mock_sch_init();
      mock_i2c_init();
      mock_inp_init();
      SLM_CONFIG config = {};
      config.address = 0x11;
      config.off_effect_mode = SLM_OFF_EFFECT_DISABLED;
      config.program_id = SLM_PROGRAM1;
      EXPECT_CALL(*i2c_mock, i2c_write(_, _, _)).WillOnce(Return(I2C_STATUS_OK));
      EXPECT_CALL(*sch_mock, sch_set_task_period(_, _));
      EXPECT_CALL(*sch_mock, sch_trigger_task(_));
      EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, TASKPRIO_LOW, _, TASKSTATE_STOPPED, TASKTYPE_TRIGGER)).WillOnce(Return(RETURN_OK));
      EXPECT_CALL(*inp_mock, inp_add_input_listener(_)).WillOnce(Return(RETURN_OK));
      EXPECT_EQ(RETURN_OK, slm_initialize(&config));
   }

   virtual void TearDown()
   {
      mock_logger_deinit();
      mock_sch_deinit();
      mock_i2c_deinit();
      mock_inp_deinit();
   }
};

/**
 * @test Initialization of LED module
 */
TEST(ledModuleInitialization, initialization)
{
   mock_logger_init();
   mock_sch_init();
   mock_i2c_init();
   mock_inp_init();

   SLM_CONFIG config = {};
   config.address = 0x11;
   config.off_effect_mode = SLM_OFF_EFFECT_ENABLED;
   config.program_id = SLM_PROGRAM1;

   /**
	 * <b>scenario</b>: Config not provided.<br>
	 * <b>expected</b>: Module not initialized.<br>
    * ************************************************
	 */
   EXPECT_EQ(RETURN_NOK, slm_initialize(nullptr));

   /**
    * <b>scenario</b>: Cannot add task to scheduler.<br>
    * <b>expected</b>: Module not initialized.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_, _, _)).WillOnce(Return(I2C_STATUS_OK));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, _));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, TASKPRIO_LOW, _, TASKSTATE_STOPPED, TASKTYPE_TRIGGER)).WillOnce(Return(RETURN_NOK));
   EXPECT_EQ(RETURN_NOK, slm_initialize(&config));

   /**
    * <b>scenario</b>: Module initialization.<br>
    * <b>expected</b>: Module initialized.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_, _, _)).WillOnce(Return(I2C_STATUS_OK));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, _));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, TASKPRIO_LOW, _, TASKSTATE_STOPPED, TASKTYPE_TRIGGER)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*inp_mock, inp_add_input_listener(_)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_OK, slm_initialize(&config));

   mock_logger_deinit();
   mock_sch_deinit();
   mock_i2c_deinit();
   mock_inp_deinit();
}

/**
 * @test Start defined led program with OFF effect
 */
TEST_F(ledFixture, start_led_program)
{
   uint8_t PROGRAM1_STEP_PERIOD = 20;
   uint8_t PROGRAM1_OFF_EFFECT_PERIOD = 200;
   uint16_t PROGRAM1_LIGHT_TIME = 20000;
   /**
    * <b>scenario</b>: LED program started - without always on, with OFF effect.<br>
    * <b>expected</b>: Correct I2C data sequence.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   EXPECT_EQ(RETURN_OK, slm_start_program());
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* step 2 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* step 3 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 4 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 5 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 6 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 7 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 8 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());


   /* timeout reached, off effect should be started */
   /* off effect step 1 */
   EXPECT_CALL(*inp_mock, inp_get(_)).WillOnce(Return(INPUT_STATE_INACTIVE));
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 2 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 3 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 4 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 5 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 6 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 7 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 8 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 9 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 10 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT_READY, slm_get_state());

   /* OFF effect ready, disabling leds */

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x00),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, 0));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF, slm_get_state());

}

/**
 * @test OFF effect is running, sensor is activated
 */
TEST_F(ledFixture, start_led_program_sensor_gets_acitvated_during_off_effect)
{
   uint8_t PROGRAM1_STEP_PERIOD = 20;
   uint8_t PROGRAM1_OFF_EFFECT_PERIOD = 200;
   uint16_t PROGRAM1_LIGHT_TIME = 20000;
   /**
    * <b>scenario</b>: LED program started - sensor active on timeout.<br>
    * <b>expected</b>: Tim.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   EXPECT_EQ(RETURN_OK, slm_start_program());
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* step 2 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* step 3 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 4 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 5 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 6 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 7 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 8 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());


   /* timeout reached, off effect should be started */
   /* off effect step 1 */
   EXPECT_CALL(*inp_mock, inp_get(_)).WillOnce(Return(INPUT_STATE_INACTIVE));
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 2 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   INPUT_STATUS inp_status = {};
   inp_status.id = INPUT_STAIRS_SENSOR;
   inp_status.state = INPUT_STATE_ACTIVE;

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_sensor_state_change(inp_status);
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());


   /* timeout reached, off effect should be started */
   /* off effect step 1 */
   EXPECT_CALL(*inp_mock, inp_get(_)).WillOnce(Return(INPUT_STATE_INACTIVE));
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 2 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 3 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 4 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 5 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 6 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 7 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 8 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 9 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   /* off effect step 10 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT_READY, slm_get_state());

   /* OFF effect ready, disabling leds */

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x00),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, 0));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF, slm_get_state());

}

/**
 * @test Start defined led program without OFF effect
 */
TEST_F(ledFixtureNoEffect, start_led_program)
{
   uint8_t PROGRAM1_STEP_PERIOD = 20;
   uint16_t PROGRAM1_LIGHT_TIME = 20000;
   /**
    * <b>scenario</b>: LED program started - without always on, without OFF effect.<br>
    * <b>expected</b>: Correct I2C data sequence.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   EXPECT_EQ(RETURN_OK, slm_start_program());
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* step 2 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* step 3 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 4 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 5 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 6 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 7 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 8 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());

   EXPECT_CALL(*inp_mock, inp_get(_)).WillOnce(Return(INPUT_STATE_INACTIVE));
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x00),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, 0));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF, slm_get_state());

}

/**
 * @test Start defined led program without OFF effect
 */
TEST_F(ledFixtureNoEffect, start_led_program_sensor_acitve_on_timeout)
{
   uint8_t PROGRAM1_STEP_PERIOD = 20;
   uint16_t PROGRAM1_LIGHT_TIME = 20000;
   /**
    * <b>scenario</b>: LED program running, on timeout sensor is still active.<br>
    * <b>expected</b>: Timeout counter should be restarted.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   EXPECT_EQ(RETURN_OK, slm_start_program());
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* step 2 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* step 3 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 4 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 5 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 6 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 7 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 8 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());


   EXPECT_CALL(*inp_mock, inp_get(_)).WillOnce(Return(INPUT_STATE_ACTIVE));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   slm_on_timeout();

   EXPECT_CALL(*inp_mock, inp_get(_)).WillOnce(Return(INPUT_STATE_INACTIVE));
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x00),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, 0));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF, slm_get_state());

}

/**
 * @test Start program with AlwaysOn mode
 */
TEST_F(ledFixtureNoEffect, start_led_alw_on)
{
   uint8_t PROGRAM1_STEP_PERIOD = 20;
   uint16_t PROGRAM1_LIGHT_TIME = 20000;
   /**
    * <b>scenario</b>: LED program started - with always on.<br>
    * <b>expected</b>: Correct I2C data sequence.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   EXPECT_EQ(RETURN_OK, slm_start_program_alw_on());
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_EQ(SLM_PROGRAM1, slm_get_current_program_id());

   /* step 2 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 3 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 4 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 5 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 6 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 7 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 8 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());


   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   EXPECT_EQ(RETURN_OK, slm_stop_program());
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x00),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, 0));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF, slm_get_state());

}

/**
 * @test Start LED animation on sensor state change
 */
TEST_F(ledFixtureNoEffect, led_on_sensor_change)
{
   uint8_t PROGRAM1_STEP_PERIOD = 20;
   uint16_t PROGRAM1_LIGHT_TIME = 20000;
   INPUT_STATUS inp_status {};
   inp_status.id = INPUT_STAIRS_SENSOR;
   inp_status.state = INPUT_STATE_ACTIVE;
   /**
    * <b>scenario</b>: LED program started - without always on, without OFF effect.<br>
    * <b>expected</b>: Correct I2C data sequence.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_sensor_state_change(inp_status);
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* step 2 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* step 3 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 4 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 5 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 6 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 7 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 8 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());

   EXPECT_CALL(*inp_mock, inp_get(_)).WillOnce(Return(INPUT_STATE_INACTIVE));
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x00),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, 0));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF, slm_get_state());

   /**
    * <b>scenario</b>: Sensor state ON received during leds switching off.<br>
    * <b>expected</b>: LEDs enabled again.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_sensor_state_change(inp_status);
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* step 2 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* step 3 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 4 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 5 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 6 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 7 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 8 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());

   EXPECT_CALL(*inp_mock, inp_get(_)).WillOnce(Return(INPUT_STATE_INACTIVE));
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_sensor_state_change(inp_status);
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 7 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   /* step 8 */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());

   EXPECT_CALL(*inp_mock, inp_get(_)).WillOnce(Return(INPUT_STATE_INACTIVE));
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x3F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x1F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_OFF, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x00),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, 0));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF, slm_get_state());
}

