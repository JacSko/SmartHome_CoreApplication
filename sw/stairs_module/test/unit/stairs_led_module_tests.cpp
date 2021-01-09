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


   /**
    * <b>scenario</b>: Getting config after initialization.<br>
    * <b>expected</b>: Correct config returned.<br>
    * ************************************************
    */
   SLM_CONFIG result;
   EXPECT_EQ(RETURN_OK, slm_get_config(&result));
   EXPECT_EQ(result.address, 0x11);
   EXPECT_EQ(result.off_effect_mode, SLM_OFF_EFFECT_ENABLED);
   EXPECT_EQ(result.program_id, SLM_PROGRAM1);

   /**
    * <b>scenario</b>: Getting config with empty buffer.<br>
    * <b>expected</b>: RETURN_NOK erturned.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, slm_get_config(nullptr));

   mock_logger_deinit();
   mock_sch_deinit();
   mock_i2c_deinit();
   mock_inp_deinit();
}

/**
 * @test Test of correct enabling and disabling sequence including shutdown effect
 */
TEST_F(ledFixture, start_led_program)
{
   uint8_t PROGRAM1_STEP_PERIOD = 20;
   uint8_t PROGRAM1_OFF_EFFECT_PERIOD = 200;
   uint16_t PROGRAM1_LIGHT_TIME = 20000;
   /**
    * <b>scenario</b>: LED program started via module method.<br>
    * <b>expected</b>: Correct I2C data sequence.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   EXPECT_EQ(RETURN_OK, slm_start_program());
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
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

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* last step, all leds on */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());

   /* timeout reached, off effect should be started */
   EXPECT_CALL(*inp_mock, inp_get(_)).WillOnce(Return(INPUT_STATE_INACTIVE));
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT_READY, slm_get_state());

   /* off effect done, disabling leds */
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
 * @test Shutdown effect is ongoing, sensor is activated
 */
TEST_F(ledFixture, start_led_program_sensor_acitvated_during_off_effect)
{
   uint8_t PROGRAM1_STEP_PERIOD = 20;
   uint8_t PROGRAM1_OFF_EFFECT_PERIOD = 200;
   uint16_t PROGRAM1_LIGHT_TIME = 20000;
   /**
    * <b>scenario</b>: LED program started - sensor is activated during shutdown effect.<br>
    * <b>expected</b>: All leds enabled again, than disabled after timoeut.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   EXPECT_EQ(RETURN_OK, slm_start_program());
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
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

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* last state, all leds are on */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());

   /* starting shutdown effect */
   EXPECT_CALL(*inp_mock, inp_get(_)).WillOnce(Return(INPUT_STATE_INACTIVE));
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   INPUT_STATUS inp_status = {};
   inp_status.id = INPUT_STAIRS_SENSOR;
   inp_status.state = INPUT_STATE_ACTIVE;

   /* notification with sensor state change, all leds should be enabled again */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_sensor_state_change(inp_status);
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());

   /* timeout, starting shutdown effect */
   EXPECT_CALL(*inp_mock, inp_get(_)).WillOnce(Return(INPUT_STATE_INACTIVE));
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

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
 * @test Test of correct enabling and disabling sequence without shutdown effect
 */
TEST_F(ledFixtureNoEffect, start_led_program)
{
   uint8_t PROGRAM1_STEP_PERIOD = 20;
   uint16_t PROGRAM1_LIGHT_TIME = 20000;
   /**
    * <b>scenario</b>: LED program started using module method.<br>
    * <b>expected</b>: Correct I2C data sequence.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   EXPECT_EQ(RETURN_OK, slm_start_program());
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
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

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /* all leds are enabled */
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
 * @test Test program sequence when sensor is still active on timeout
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

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
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

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());

   /* timeout occurs, but sensor is still active */
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
 * @test Test led module behavior with AlwaysOn flag
 */
TEST_F(ledFixtureNoEffect, start_led_alw_on)
{
   uint8_t PROGRAM1_STEP_PERIOD = 20;
   uint16_t PROGRAM1_LIGHT_TIME = 20000;
   /**
    * <b>scenario</b>: LED program started - with always on flag.<br>
    * <b>expected</b>: Correct I2C data sequence.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   EXPECT_EQ(RETURN_OK, slm_start_program_alw_on());
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_EQ(SLM_PROGRAM1, slm_get_current_program_id());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
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

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   /* task should be stopped due to AlwaysOn flag */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());

   /* program stopped manually */
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
 * @test Test led module behavior with AlwaysOn flag on sensor state changed
 */
TEST_F(ledFixture, start_led_alw_on_with_effect)
{
   uint8_t PROGRAM1_STEP_PERIOD = 20;
   uint8_t PROGRAM1_OFF_EFFECT_PERIOD = 200;
   uint16_t PROGRAM1_LIGHT_TIME = 20000;
   /**
    * <b>scenario</b>: LED program started on sensor state changed - with always on flag.<br>
    * <b>expected</b>: Correct I2C data sequence.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   EXPECT_EQ(RETURN_OK, slm_start_program_alw_on());
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_EQ(SLM_PROGRAM1, slm_get_current_program_id());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
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

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   /* task should be stopped due to AlwaysOn flag */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_LIGHT_TIME));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ON, slm_get_state());

   /* program stopped manually, effect started */
   /* timeout, starting shutdown effect */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   EXPECT_EQ(RETURN_OK, slm_stop_program());
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0xFF),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_OFF_EFFECT_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_OFF_EFFECT_READY, slm_get_state());

   /* effect ready, disabling leds*/
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
 * @test Start LED program on sensor state change
 */
TEST_F(ledFixtureNoEffect, led_on_sensor_change)
{
   uint8_t PROGRAM1_STEP_PERIOD = 20;
   uint16_t PROGRAM1_LIGHT_TIME = 20000;
   INPUT_STATUS inp_status {};
   inp_status.id = INPUT_STAIRS_SENSOR;
   inp_status.state = INPUT_STATE_ACTIVE;
   /**
    * <b>scenario</b>: Sensor state changed.<br>
    * <b>expected</b>: LED module program started.<br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_sensor_state_change(inp_status);
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
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

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

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
 * @test Notification with sensor state active received during led shutdown.
 */
TEST_F(ledFixtureNoEffect, led_sensor_active_during_shutdown)
{
   uint8_t PROGRAM1_STEP_PERIOD = 20;
   uint16_t PROGRAM1_LIGHT_TIME = 20000;
   INPUT_STATUS inp_status {};
   inp_status.id = INPUT_STAIRS_SENSOR;
   inp_status.state = INPUT_STATE_ACTIVE;

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

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x03),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x07),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x0F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
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

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

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
   /* notification with sensor state active, leds enabled again from current position */
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

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x7F),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_timeout();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

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

   /* timeout reached when state is OFF - task should be disabled */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_,TASKSTATE_STOPPED));
   slm_on_timeout();

}

/**
 * @test Starting module from interface when module is in incorrect state.
 */
TEST_F(ledFixtureNoEffect, led_start_when_incorrect_state)
{
   uint8_t PROGRAM1_STEP_PERIOD = 20;
   INPUT_STATUS inp_status {};
   inp_status.id = INPUT_STAIRS_SENSOR;
   inp_status.state = INPUT_STATE_ACTIVE;

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_on_sensor_state_change(inp_status);
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());

   /**
   * <b>scenario</b>: Module is in state different than OFF, module requested to start.<br>
   * <b>expected</b>: RETURN_NOK returned.<br>
   * ************************************************
   */
   EXPECT_EQ(RETURN_NOK, slm_start_program());
   EXPECT_EQ(RETURN_NOK, slm_start_program_alw_on());
}

/**
 * @test Getting program sequence by ID.
 */
TEST_F(ledFixtureNoEffect, led_get_program_by_id)
{
   SLM_PROGRAM program = {};
   /**
   * <b>scenario</b>: Buffer is NULL.<br>
   * <b>expected</b>: RETURN_NOK returned.<br>
   * ************************************************
   */
   EXPECT_EQ(RETURN_NOK, slm_get_program_by_id(SLM_PROGRAM1, nullptr));
   /**
   * <b>scenario</b>: Invalid program ID requested.<br>
   * <b>expected</b>: RETURN_NOK returned.<br>
   * ************************************************
   */
   EXPECT_EQ(RETURN_NOK, slm_get_program_by_id((SLM_PROGRAM_ID)(SLM_PROGRAM3 + 1), &program));
   /**
   * <b>scenario</b>: Requested correct program ID.<br>
   * <b>expected</b>: Buffer filled with correct program.<br>
   * ************************************************
   */
   EXPECT_EQ(RETURN_OK, slm_get_program_by_id(SLM_PROGRAM1, &program));
   EXPECT_EQ(program.program_steps_count, 9);
   EXPECT_EQ(program.off_effect_steps_count, 10);
}

/**
 * @test Replacing LED program.
 */
TEST_F(ledFixtureNoEffect, led_replace_program)
{
   SLM_PROGRAM new_program, incorrect_program;
   new_program.program_steps_count = 5;
   incorrect_program.program_steps_count = 5;
   new_program.off_effect_steps_count = 5;
   incorrect_program.off_effect_steps_count = 5;

   for (uint8_t i = 0; i < 5; i++)
   {
      new_program.off_effect_steps[i].period = 50;
      new_program.program_steps[i].period = 50;
      incorrect_program.off_effect_steps[i].period = 0;
      incorrect_program.program_steps[i].period = 0;
   }
   /**
   * <b>scenario</b>: Program is NULL.<br>
   * <b>expected</b>: RETURN_NOK returned.<br>
   * ************************************************
   */
   EXPECT_EQ(RETURN_NOK, slm_replace_program(SLM_PROGRAM1, nullptr));
   /**
   * <b>scenario</b>: Invalid program ID requested.<br>
   * <b>expected</b>: RETURN_NOK returned.<br>
   * ************************************************
   */
   EXPECT_EQ(RETURN_NOK, slm_replace_program((SLM_PROGRAM_ID)(SLM_PROGRAM3 + 1), nullptr));
   /**
   * <b>scenario</b>: New program has invalid timing.<br>
   * <b>expected</b>: RETURN_NOK returned.<br>
   * ************************************************
   */
   EXPECT_EQ(RETURN_NOK, slm_replace_program(SLM_PROGRAM1, &incorrect_program));
   /**
   * <b>scenario</b>: Changing program.<br>
   * <b>expected</b>: Program changed correctly.<br>
   * ************************************************
   */
   EXPECT_EQ(RETURN_OK, slm_replace_program(SLM_PROGRAM2, &new_program));
   /**
   * <b>scenario</b>: Changing current program when program is running.<br>
   * <b>expected</b>: RETURN_NOK returned.<br>
   * ************************************************
   */
   uint8_t PROGRAM1_STEP_PERIOD = 20;

   EXPECT_CALL(*i2c_mock, i2c_write(_,I2C_DATA_MATCH(0x01),_));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, PROGRAM1_STEP_PERIOD));
   slm_start_program();
   EXPECT_EQ(SLM_STATE_ONGOING_ON, slm_get_state());
   EXPECT_EQ(RETURN_NOK, slm_replace_program(SLM_PROGRAM1, &new_program));
}
