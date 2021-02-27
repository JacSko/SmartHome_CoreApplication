#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/inputs_board.c"
#include "../../../../ext_lib/CMSIS/stubs/device/stm32f4xx.h"
#include "inputs_interrupt_handler.h"
#ifdef __cplusplus
}
#endif
#include "i2c_driver_mock.h"
#include "logger_mock.h"
#include "gpio_lib_mock.h"
#include "task_scheduler_mock.h"

/* ============================= */
/**
 * @file inputs_board_tests.cpp
 *
 * @brief Unit tests of inputs board handler
 *
 * @details
 * This tests verifies behavior of inputs board handler
 *
 * @author Jacek Skowronek
 * @date 22/12/2020
 */
/* ============================= */


using namespace ::testing;

struct callbackMock
{
	MOCK_METHOD1(callback, void(INPUT_STATUS));
};

callbackMock* callMock;

void fake_callback(INPUT_STATUS state)
{
	callMock->callback(state);
}

MATCHER_P(INPUT_STATUS_MATCH, expected, "")
{
   return expected.id == arg.id && expected.state == arg.state;
}

struct inputsBoardFixture : public ::testing::Test
{
	virtual void SetUp()
	{
	   cfg.address = 0x10;
	   cfg.items[0] = {INPUT_WARDROBE_LED, 1};
	   cfg.items[1] = {INPUT_BATHROOM_LED, 2};
	   cfg.items[2] = {INPUT_SOCKETS, 9};
	   cfg.items[3] = {INPUT_BEDROOM_AC, 10};
	   cfg.items[4] = {INPUT_WARDROBE_AC, 11};
	   cfg.items[5] = {INPUT_KITCHEN_AC, 12};
	   cfg.items[6] = {INPUT_BATHROOM_AC, 13};
	   cfg.items[7] = {INPUT_STAIRS_AC, 14};
	   cfg.items[8] = {INPUT_STAIRS_SENSOR, 15};
	   cfg.items[9] = {INPUT_KITCHEN_WALL, 16};
	   mock_i2c_init();
	   mock_logger_init();
		stm_stub_init();
		mock_gpio_init();
		mock_sch_init();
		callMock = new callbackMock();

      EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, _, _, _, _)).WillOnce(Return(RETURN_OK))
                                                                  .WillOnce(Return(RETURN_OK));
      EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_, PB4, gpio_mode_in_floating));
      EXPECT_EQ(RETURN_OK, inp_initialize(&cfg));

	}

	virtual void TearDown()
	{

	   EXPECT_CALL(*sch_mock, sch_unsubscribe(_)).Times(2);
	   inp_deinitialize();
	   mock_i2c_deinit();
	   mock_logger_deinit();
		stm_stub_deinit();
		mock_gpio_deinit();
		mock_sch_deinit();
		delete callMock;
	}

   INPUTS_CONFIG cfg;
};

/**
 * @test Initialization test of input board
 */
TEST(inputsBoardTests, initialization)
{

   mock_i2c_init();
   mock_logger_init();
   stm_stub_init();
   mock_gpio_init();
   mock_sch_init();

   INPUTS_CONFIG cfg;
   cfg.address = 0x10;
   cfg.items[0] = {INPUT_WARDROBE_LED, 1};
   cfg.items[1] = {INPUT_BATHROOM_LED, 2};
   cfg.items[2] = {INPUT_SOCKETS, 9};
   cfg.items[3] = {INPUT_BEDROOM_AC, 10};
   cfg.items[4] = {INPUT_WARDROBE_AC, 11};
   cfg.items[5] = {INPUT_KITCHEN_AC, 12};
   cfg.items[6] = {INPUT_BATHROOM_AC, 13};
   cfg.items[7] = {INPUT_STAIRS_AC, 14};
   cfg.items[8] = {INPUT_STAIRS_SENSOR, 15};
   cfg.items[9] = {INPUT_KITCHEN_WALL, 16};

   /**
    * <b>scenario</b>: Module initialization requested, no config provided <br>
    * <b>expected</b>: Module not initialized <br>
    * ************************************************
    */
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_, PB4, gpio_mode_in_floating));
   EXPECT_EQ(RETURN_NOK, inp_initialize(NULL));

   /**
    * <b>scenario</b>: Module initialization requested, cannot add scheduler tasks <br>
    * <b>expected</b>: Module not initialized <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, _, _, _, _)).WillOnce(Return(RETURN_NOK));
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_, PB4, gpio_mode_in_floating));
   EXPECT_EQ(RETURN_NOK, inp_initialize(&cfg));

   /**
    * <b>scenario</b>: Module initialization requested, correct sequence <br>
    * <b>expected</b>: Module initialized <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, _, _, _, _)).WillOnce(Return(RETURN_OK))
                                                               .WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_, PB4, gpio_mode_in_floating));
   EXPECT_EQ(RETURN_OK, inp_initialize(&cfg));

   /**
    * <b>scenario</b>: Getting inputs config <br>
    * <b>expected</b>: Correct config returned <br>
    * ************************************************
    */
   INPUTS_CONFIG result;
   EXPECT_EQ(RETURN_ERROR, inp_get_config(nullptr));
   EXPECT_EQ(RETURN_OK, inp_get_config(&result));
   EXPECT_EQ(result.address, 0x10);

   mock_i2c_deinit();
   mock_logger_deinit();
   stm_stub_deinit();
   mock_gpio_deinit();
   mock_sch_deinit();

}

/**
 * @test Periodic inputs read
 */
TEST_F(inputsBoardFixture, inputs_read_periodic_event)
{
   EXPECT_EQ(RETURN_OK, inp_add_input_listener(&fake_callback));
   /**
    * <b>scenario</b>: Inputs read on autoupdate timeout, cannot read i2c data <br>
    * <b>expected</b>: Callbacks not called <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Return(I2C_STATUS_ERROR));
   EXPECT_CALL(*callMock, callback(_)).Times(0);
   inp_read_inputs();

   /**
    * <b>scenario</b>: Inputs read on autoupdate timeout, wardrobe led in turned on <br>
    * <b>expected</b>: Callbacks called <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&]
                                                   (I2C_ADDRESS addr, uint8_t* buf, uint8_t size) -> I2C_STATUS
                                                   {
                                                      EXPECT_EQ(addr, cfg.address+1);
                                                      EXPECT_EQ(size, 2);
                                                      buf[0] = 0xFF;
                                                      buf[1] = 0x7F;
                                                      return I2C_STATUS_OK;
                                                   }));
   EXPECT_CALL(*callMock, callback(INPUT_STATUS_MATCH(INPUT_STATUS{INPUT_WARDROBE_LED, INPUT_STATE_ACTIVE})));
   inp_read_inputs();
   EXPECT_EQ(INPUT_STATE_ACTIVE, inp_get(INPUT_WARDROBE_LED));

   /**
    * <b>scenario</b>: Inputs read on autoupdate timeout, wardrobe led not changed <br>
    * <b>expected</b>: Callbacks not called <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&]
                                                   (I2C_ADDRESS addr, uint8_t* buf, uint8_t size) -> I2C_STATUS
                                                   {
                                                      EXPECT_EQ(addr, cfg.address+1);
                                                      EXPECT_EQ(size, 2);
                                                      buf[0] = 0xFF;
                                                      buf[1] = 0x7F;
                                                      return I2C_STATUS_OK;
                                                   }));
   EXPECT_CALL(*callMock, callback(_)).Times(0);
   inp_read_inputs();

   /**
    * <b>scenario</b>: Inputs read on autoupdate timeout, wardrobe led turned off <br>
    * <b>expected</b>: Callbacks not called <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&]
                                                   (I2C_ADDRESS addr, uint8_t* buf, uint8_t size) -> I2C_STATUS
                                                   {
                                                      EXPECT_EQ(addr, cfg.address+1);
                                                      EXPECT_EQ(size, 2);
                                                      buf[0] = 0xFF;
                                                      buf[1] = 0xFF;
                                                      return I2C_STATUS_OK;
                                                   }));
   EXPECT_CALL(*callMock, callback(INPUT_STATUS_MATCH(INPUT_STATUS{INPUT_WARDROBE_LED, INPUT_STATE_INACTIVE})));
   inp_read_inputs();
   EXPECT_EQ(INPUT_STATE_INACTIVE, inp_get(INPUT_WARDROBE_LED));
}

/**
 * @test Interrupt inputs read
 */
TEST_F(inputsBoardFixture, inputs_read_interrupt_event)
{
   EXPECT_EQ(RETURN_OK, inp_add_input_listener(&fake_callback));
   /**
    * <b>scenario</b>: Inputs read interrupt, kitchen light turned on <br>
    * <b>expected</b>: Callbacks called <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_trigger_task(_));

   inp_on_interrupt_recevied(); /* simulate interrupt */

   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&]
                                                   (I2C_ADDRESS addr, uint8_t* buf, uint8_t size) -> I2C_STATUS
                                                   {
                                                      EXPECT_EQ(addr, cfg.address+1);
                                                      EXPECT_EQ(size, 2);
                                                      buf[0] = 0xF7;
                                                      buf[1] = 0xFF;
                                                      return I2C_STATUS_OK;
                                                   }));
   EXPECT_CALL(*callMock, callback(INPUT_STATUS_MATCH(INPUT_STATUS{INPUT_KITCHEN_AC, INPUT_STATE_ACTIVE})));
   inp_on_timeout();
   EXPECT_EQ(INPUT_STATE_ACTIVE, inp_get(INPUT_KITCHEN_AC));

  /**
   * <b>scenario</b>: Inputs read interrupt, kitchen light turned off <br>
   * <b>expected</b>: Callbacks called <br>
   * ************************************************
   */
  EXPECT_CALL(*sch_mock, sch_trigger_task(_));

  inp_on_interrupt_recevied(); /* simulate interrupt */

  EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&]
                                                  (I2C_ADDRESS addr, uint8_t* buf, uint8_t size) -> I2C_STATUS
                                                  {
                                                     EXPECT_EQ(addr, cfg.address+1);
                                                     EXPECT_EQ(size, 2);
                                                     buf[0] = 0xFF;
                                                     buf[1] = 0xFF;
                                                     return I2C_STATUS_OK;
                                                  }));
  EXPECT_CALL(*callMock, callback(INPUT_STATUS_MATCH(INPUT_STATUS{INPUT_KITCHEN_AC, INPUT_STATE_INACTIVE})));
  inp_on_timeout();
  EXPECT_EQ(INPUT_STATE_INACTIVE, inp_get(INPUT_KITCHEN_AC));
}

/**
 * @test Changing the autoupdate period
 */
TEST_F(inputsBoardFixture, inputs_autoupdate_period_change)
{
   /**
    * <b>scenario</b>: Changing autoupdate period to incorrect value <br>
    * <b>expected</b>: Period not changed <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, inp_set_periodic_update_time(INPUTS_AUTOUPDATE_MIN_TIME_MS - 1));
   EXPECT_EQ(RETURN_NOK, inp_set_periodic_update_time(INPUTS_AUTOUPDATE_MAX_TIME_MS + 1));

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,_)).WillOnce(Return(RETURN_NOK));
   EXPECT_EQ(RETURN_NOK, inp_set_periodic_update_time(INPUTS_AUTOUPDATE_DEF_TIME_MS));

   /**
    * <b>scenario</b>: Changing autoupdate period to correct value <br>
    * <b>expected</b>: Period changed <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_period(_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_OK, inp_set_periodic_update_time(INPUTS_AUTOUPDATE_DEF_TIME_MS - 10));
   EXPECT_EQ(INPUTS_AUTOUPDATE_DEF_TIME_MS -10, inp_get_periodic_update_time());
}

/**
 * @test Changing the debounce period
 */
TEST_F(inputsBoardFixture, inputs_debounce_period_change)
{
   /**
    * <b>scenario</b>: Changing debounce period to incorrect value <br>
    * <b>expected</b>: Period not changed <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, inp_set_debounce_time(INPUTS_DEBOUNCE_MIN_TIME_MS - 1));
   EXPECT_EQ(RETURN_NOK, inp_set_debounce_time(INPUTS_DEBOUNCE_MAX_TIME_MS + 1));

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,_)).WillOnce(Return(RETURN_NOK));
   EXPECT_EQ(RETURN_NOK, inp_set_debounce_time(INPUTS_DEBOUNCE_DEF_TIME_MS));

   /**
    * <b>scenario</b>: Changing debounce period to correct value <br>
    * <b>expected</b>: Period changed <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_period(_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_OK, inp_set_debounce_time(INPUTS_DEBOUNCE_DEF_TIME_MS - 10));
   EXPECT_EQ(INPUTS_DEBOUNCE_DEF_TIME_MS -10, inp_get_debounce_time());
}

/**
 * @test Switching ON/OFF inputs autoupdate
 */
TEST_F(inputsBoardFixture, inputs_autoupdate_on_off)
{
   /**
    * <b>scenario</b>: Switching autoupdate to OFF <br>
    * <b>expected</b>: Scheduler task stopped <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_,TASKSTATE_STOPPED));
   inp_disable_periodic_update();

   /**
    * <b>scenario</b>: Switching autoupdate to ON <br>
    * <b>expected</b>: Scheduler task stopped <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_,TASKSTATE_RUNNING));
   inp_enable_periodic_update();

}

/**
 * @test Switching ON/OFF interrupt handling
 */
TEST_F(inputsBoardFixture, inputs_interrupt_on_off)
{
   /**
    * <b>scenario</b>: Switching interrupt handling to OFF <br>
    * <b>expected</b>: Scheduler task not started on interrupt <br>
    * ************************************************
    */
   inp_disable_interrupt();

   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(0);
   inp_on_interrupt_recevied();

   /**
    * <b>scenario</b>: Switching interrupt handling to ON <br>
    * <b>expected</b>: Scheduler task started on interrupt <br>
    * ************************************************
    */
   inp_enable_interrupt();

   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(1);
   inp_on_interrupt_recevied();
}

/**
 * @test Adding/removing state listeners
 */
TEST_F(inputsBoardFixture, inputs_listeners_add_remove)
{

   /**
    * <b>scenario</b>: Two listeners added, new data received <br>
    * <b>expected</b>: 2 callbacks called <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_OK, inp_add_input_listener(&fake_callback));
   EXPECT_EQ(RETURN_OK, inp_add_input_listener(&fake_callback));

   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&]
                                                   (I2C_ADDRESS addr, uint8_t* buf, uint8_t size) -> I2C_STATUS
                                                   {
                                                      EXPECT_EQ(addr, cfg.address+1);
                                                      EXPECT_EQ(size, 2);
                                                      buf[0] = 0x7F;
                                                      buf[1] = 0xFF;
                                                      return I2C_STATUS_OK;
                                                   }));
   EXPECT_CALL(*callMock, callback(INPUT_STATUS_MATCH(INPUT_STATUS{INPUT_KITCHEN_WALL, INPUT_STATE_ACTIVE}))).Times(2);
   inp_read_inputs();

   /**
    * <b>scenario</b>: One listener removed, new data received <br>
    * <b>expected</b>: 1 callback called <br>
    * ************************************************
    */
   inp_remove_input_listener(&fake_callback);

   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&]
                                                   (I2C_ADDRESS addr, uint8_t* buf, uint8_t size) -> I2C_STATUS
                                                   {
                                                      EXPECT_EQ(addr, cfg.address+1);
                                                      EXPECT_EQ(size, 2);
                                                      buf[0] = 0xFF;
                                                      buf[1] = 0xFF;
                                                      return I2C_STATUS_OK;
                                                   }));
   EXPECT_CALL(*callMock, callback(INPUT_STATUS_MATCH(INPUT_STATUS{INPUT_KITCHEN_WALL, INPUT_STATE_INACTIVE}))).Times(1);
   inp_read_inputs();

}

/**
 * @test Change of input which is not included in config
 */
TEST_F(inputsBoardFixture, inputs_change_not_configured_input)
{
   EXPECT_EQ(RETURN_OK, inp_add_input_listener(&fake_callback));
   /**
    * <b>scenario</b>: Input not configured in in config changed <br>
    * <b>expected</b>: Callbacks not called <br>
    * ************************************************
    */

   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&]
                                                   (I2C_ADDRESS addr, uint8_t* buf, uint8_t size) -> I2C_STATUS
                                                   {
                                                      EXPECT_EQ(addr, cfg.address+1);
                                                      EXPECT_EQ(size, 2);
                                                      buf[0] = 0xFF;
                                                      buf[1] = 0xFE;
                                                      return I2C_STATUS_OK;
                                                   }));
   EXPECT_CALL(*callMock, callback(_)).Times(0);
   inp_read_inputs();
}

/**
 * @test More than one input changed in the same time
 */
TEST_F(inputsBoardFixture, inputs_change_the_same_time)
{
   EXPECT_EQ(RETURN_OK, inp_add_input_listener(&fake_callback));
   /**
    * <b>scenario</b>: Three inputs changed in the same time <br>
    * <b>expected</b>: Callbacks called three times <br>
    * ************************************************
    */

   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&]
                                                   (I2C_ADDRESS addr, uint8_t* buf, uint8_t size) -> I2C_STATUS
                                                   {
                                                      EXPECT_EQ(addr, cfg.address+1);
                                                      EXPECT_EQ(size, 2);
                                                      buf[0] = 0xF8;
                                                      buf[1] = 0xFF;
                                                      return I2C_STATUS_OK;
                                                   }));
   EXPECT_CALL(*callMock, callback(INPUT_STATUS_MATCH(INPUT_STATUS{INPUT_SOCKETS, INPUT_STATE_ACTIVE})));
   EXPECT_CALL(*callMock, callback(INPUT_STATUS_MATCH(INPUT_STATUS{INPUT_BEDROOM_AC, INPUT_STATE_ACTIVE})));
   EXPECT_CALL(*callMock, callback(INPUT_STATUS_MATCH(INPUT_STATUS{INPUT_WARDROBE_AC, INPUT_STATE_ACTIVE})));
   inp_read_inputs();

   INPUT_STATUS result [INPUTS_MAX_INPUT_LINES];
   EXPECT_EQ(RETURN_ERROR, inp_get_all(nullptr));
   EXPECT_EQ(RETURN_OK, inp_get_all(result));

   EXPECT_EQ(result[0].id, INPUT_WARDROBE_LED);
   EXPECT_EQ(result[0].state, INPUT_STATE_INACTIVE);

   EXPECT_EQ(result[2].id, INPUT_SOCKETS);
   EXPECT_EQ(result[2].state, INPUT_STATE_ACTIVE);

   EXPECT_EQ(result[3].id, INPUT_BEDROOM_AC);
   EXPECT_EQ(result[3].state, INPUT_STATE_ACTIVE);

   EXPECT_EQ(result[4].id, INPUT_WARDROBE_AC);
   EXPECT_EQ(result[4].state, INPUT_STATE_ACTIVE);
}

/**
 * @test Reading all inputs state directly from board
 */
TEST_F(inputsBoardFixture, inputs_read_all_tests)
{
   EXPECT_EQ(RETURN_OK, inp_add_input_listener(&fake_callback));
   EXPECT_CALL(*callMock, callback(_)).Times(0);
   INPUT_STATUS results [INPUTS_MAX_INPUT_LINES];
   /**
    * <b>scenario</b>: No buffer provided <br>
    * <b>expected</b>: RETURN_NOK returned <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, inp_read_all(nullptr));

   /**
    * <b>scenario</b>: Cannot read I2C data <br>
    * <b>expected</b>: RETURN_NOK returned <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Return(I2C_STATUS_ERROR));
   EXPECT_EQ(RETURN_NOK, inp_read_all(results));

   /**
    * <b>scenario</b>: Data from board read correctly <br>
    * <b>expected</b>: RETURN_OK returned <br>
    * ************************************************
    */

   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&]
                                                   (I2C_ADDRESS addr, uint8_t* buf, uint8_t size) -> I2C_STATUS
                                                   {
                                                      EXPECT_EQ(addr, cfg.address+1);
                                                      EXPECT_EQ(size, 2);
                                                      buf[0] = 0xF8;
                                                      buf[1] = 0xFF;
                                                      return I2C_STATUS_OK;
                                                   }));

   EXPECT_EQ(RETURN_OK, inp_read_all(results));
   EXPECT_EQ(results[2].id, INPUT_SOCKETS);
   EXPECT_EQ(results[2].state, INPUT_STATE_ACTIVE);
   EXPECT_EQ(results[3].id, INPUT_BEDROOM_AC);
   EXPECT_EQ(results[3].state, INPUT_STATE_ACTIVE);
   EXPECT_EQ(results[4].id, INPUT_WARDROBE_AC);
   EXPECT_EQ(results[4].state, INPUT_STATE_ACTIVE);

}
