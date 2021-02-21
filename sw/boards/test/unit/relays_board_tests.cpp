#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/relays_board.c"
#ifdef __cplusplus
}
#endif
#include "i2c_driver_mock.h"
#include "logger_mock.h"
#include "task_scheduler_mock.h"

/* ============================= */
/**
 * @file relays_board_tests.cpp
 *
 * @brief Unit tests of relays board handler
 *
 * @details
 * This tests verifies behavior of relays board handler
 *
 * @author Jacek Skowronek
 * @date 23/12/2020
 */
/* ============================= */


using namespace ::testing;

struct callbackMock
{
   MOCK_METHOD1(callback, void(const RELAY_STATUS*));
};

callbackMock* callMock;

void fake_callback(const RELAY_STATUS* state)
{
   callMock->callback(state);
}

MATCHER_P(RELAYS_STATUS_MATCH, expected, "")
{
   return expected.id == arg->id && expected.state == arg->state;
}

struct relaysBoardFixture : public ::testing::Test
{
	virtual void SetUp()
	{
	   cfg.address = 0x10;
	   cfg.items[0] = {RELAY_WARDROBE_LED, 1};
	   cfg.items[1] = {RELAY_BATHROOM_LED, 2};
	   cfg.items[2] = {RELAY_STAIRCASE_LED, 8};
	   cfg.items[3] = {RELAY_SOCKETS, 9};
	   cfg.items[4] = {RELAY_BATHROOM_FAN, 10};
	   cfg.items[5] = {RELAY_KITCHEN_WALL, 11};
	   cfg.items[6] = {RELAY_STAIRCASE_AC, 12};
	   cfg.items[7] = {RELAY_BATHROOM_AC, 13};
	   cfg.items[8] = {RELAY_KITCHEN_AC, 14};
	   cfg.items[9] = {RELAY_BEDROOM_AC, 15};
	   cfg.items[10] = {RELAY_WARDROBE_AC, 16};
	   mock_i2c_init();
	   mock_logger_init();
		mock_sch_init();
		callMock = new callbackMock();

      EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, _, _, _, _)).WillOnce(Return(RETURN_OK));
      EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&](I2C_ADDRESS addr , uint8_t* buf, uint8_t size) -> I2C_STATUS
            {
               EXPECT_EQ(size, 2);
               EXPECT_EQ(cfg.address + 1, addr);
               buf[0] = 0xFF;
               buf[1] = 0xFF;
               return I2C_STATUS_OK;
            }));
      EXPECT_EQ(RETURN_OK, rel_initialize(&cfg));

	}

	virtual void TearDown()
	{

	   EXPECT_CALL(*sch_mock, sch_unsubscribe(_)).Times(1);
	   rel_deinitialize();
	   mock_i2c_deinit();
	   mock_logger_deinit();
		mock_sch_deinit();
		delete callMock;
	}
   RELAYS_CONFIG cfg;
};

/**
 * @test Initialization test of input board
 */
TEST(relaysBoardTests, initialization)
{

   mock_i2c_init();
   mock_logger_init();
   mock_sch_init();

   RELAYS_CONFIG cfg;
   cfg.address = 0x10;
   cfg.items[0] = {RELAY_WARDROBE_LED, 1};
   cfg.items[1] = {RELAY_BATHROOM_LED, 2};
   cfg.items[2] = {RELAY_STAIRCASE_LED, 8};
   cfg.items[3] = {RELAY_SOCKETS, 9};
   cfg.items[4] = {RELAY_BATHROOM_FAN, 10};
   cfg.items[5] = {RELAY_KITCHEN_WALL, 11};
   cfg.items[6] = {RELAY_STAIRCASE_AC, 12};
   cfg.items[7] = {RELAY_BATHROOM_AC, 13};
   cfg.items[8] = {RELAY_KITCHEN_AC, 14};
   cfg.items[9] = {RELAY_BEDROOM_AC, 15};
   cfg.items[10] = {RELAY_WARDROBE_AC, 16};

   /**
    * <b>scenario</b>: Module initialization requested, no config provided <br>
    * <b>expected</b>: Module not initialized <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, rel_initialize(nullptr));

   /**
    * <b>scenario</b>: Module initialization requested, cannot add task to scheduler <br>
    * <b>expected</b>: Module not initialized <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, _, _, _, _)).WillOnce(Return(RETURN_NOK));
   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&](I2C_ADDRESS addr , uint8_t* buf, uint8_t size) -> I2C_STATUS
         {
            EXPECT_EQ(size, 2);
            EXPECT_EQ(cfg.address + 1, addr);
            buf[0] = 0xFF;
            buf[1] = 0xFF;
            return I2C_STATUS_OK;
         }));
   EXPECT_EQ(RETURN_NOK, rel_initialize(&cfg));

   /**
    * <b>scenario</b>: Module initialization requested, correct sequence <br>
    * <b>expected</b>: Module initialized <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, _, _, _, _)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&](I2C_ADDRESS addr , uint8_t* buf, uint8_t size) -> I2C_STATUS
         {
            EXPECT_EQ(size, 2);
            EXPECT_EQ(cfg.address + 1, addr);
            buf[0] = 0xFF;
            buf[1] = 0xFF;
            return I2C_STATUS_OK;
         }));
   EXPECT_EQ(RETURN_OK, rel_initialize(&cfg));


   RELAYS_CONFIG result = {};

   /**
    * <b>scenario</b>: Get module config - buffer is null <br>
    * <b>expected</b>: RETURN_NOK returned <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, rel_get_config(nullptr));
   /**
    * <b>scenario</b>: Get module config <br>
    * <b>expected</b>: Correct config returned <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_OK, rel_get_config(&result));

   EXPECT_EQ(result.address, 0x10);
   EXPECT_EQ(result.items[10].id, RELAY_WARDROBE_AC);
   EXPECT_EQ(result.items[10].relay_no, 16);

   mock_i2c_deinit();
   mock_logger_deinit();
   mock_sch_deinit();

}

/**
 * @test Setting one relay test. When one relay is going to be set, other should not be changed.
 */
TEST_F(relaysBoardFixture, relay_set_unset_tests)
{
   EXPECT_EQ(RETURN_OK, rel_add_listener(&fake_callback));

   /**
    * <b>scenario</b>: Set relay - incorrect ID the relay <br>
    * <b>expected</b>: I2C message not send <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,_,_)).Times(0);
   EXPECT_EQ(RETURN_NOK, rel_set(RELAY_ID_ENUM_MAX, RELAY_STATE_ON));
   /**
    * <b>scenario</b>: Set relay - incorrect relay state <br>
    * <b>expected</b>: I2C message not send <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,_,_)).Times(0);
   EXPECT_EQ(RETURN_NOK, rel_set(RELAY_BATHROOM_AC, RELAY_STATE_ENUM_MAX));

   /**
    * <b>scenario</b>: Set relay - cannot sent I2C data <br>
    * <b>expected</b>: RETURN_NOK returned <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,_,_)).WillOnce(Return(I2C_STATUS_ERROR));
   EXPECT_EQ(RETURN_NOK, rel_set(RELAY_BATHROOM_AC, RELAY_STATE_ON));

   /**
    * <b>scenario</b>: Set relay state to OFF when it is already OFF <br>
    * <b>expected</b>: Correct I2C message sent <br>
    * ************************************************
    */
   ASSERT_EQ(rel_get(RELAY_BATHROOM_FAN), RELAY_STATE_OFF);
   EXPECT_CALL(*i2c_mock, i2c_write(_,_,_)).WillOnce(Invoke(
         [&](I2C_ADDRESS addr, const uint8_t* data, uint8_t size) -> I2C_STATUS
         {
            EXPECT_EQ(addr, 0x10);
            EXPECT_EQ(0xFFFF, *((uint16_t*)data));
            EXPECT_EQ(size, 2U);
            return I2C_STATUS_OK;
         }));
   EXPECT_EQ(RETURN_OK, rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_OFF));

   /**
    * <b>scenario</b>: Set relay - setting RELAY_BATHROOM_AC to ON <br>
    * <b>expected</b>: Relay 13 set to ON <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,_,_)).WillOnce(Invoke(
         [&](I2C_ADDRESS addr, const uint8_t* data, uint8_t size) -> I2C_STATUS
         {
            EXPECT_EQ(addr, 0x10);
            EXPECT_EQ(0xEFFF, *((uint16_t*)data));
            EXPECT_EQ(size, 2U);
            return I2C_STATUS_OK;
         }));
   EXPECT_CALL(*callMock, callback(RELAYS_STATUS_MATCH(RELAY_STATUS{RELAY_BATHROOM_AC, RELAY_STATE_ON})));
   EXPECT_EQ(RETURN_OK, rel_set(RELAY_BATHROOM_AC, RELAY_STATE_ON));
   EXPECT_EQ(RELAY_STATE_ON, rel_get(RELAY_BATHROOM_AC));
   EXPECT_EQ(RELAY_STATE_OFF, rel_get(RELAY_BATHROOM_LED));

   /**
    * <b>scenario</b>: Set relay - setting Active relay to ON again<br>
    * <b>expected</b>: Correct message sent <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,_,_)).WillOnce(Invoke(
         [&](I2C_ADDRESS addr, const uint8_t* data, uint8_t size) -> I2C_STATUS
         {
            EXPECT_EQ(addr, 0x10);
            EXPECT_EQ(0xEFFF, *((uint16_t*)data));
            EXPECT_EQ(size, 2U);
            return I2C_STATUS_OK;
         }));
   EXPECT_EQ(RETURN_OK, rel_set(RELAY_BATHROOM_AC, RELAY_STATE_ON));
   EXPECT_EQ(RELAY_STATE_ON, rel_get(RELAY_BATHROOM_AC));
   EXPECT_EQ(RELAY_STATE_OFF, rel_get(RELAY_BATHROOM_LED));

   /**
    * <b>scenario</b>: Set relay - setting RELAY_BATHROOM_LED to ON <br>
    * <b>expected</b>: Relay 2 set to ON, previous relay unchanged <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,_,_)).WillOnce(Invoke(
         [&](I2C_ADDRESS addr, const uint8_t* data, uint8_t size) -> I2C_STATUS
         {
            EXPECT_EQ(addr, 0x10);
            EXPECT_EQ(0xEFFD, *((uint16_t*)data));
            EXPECT_EQ(size, 2U);
            return I2C_STATUS_OK;
         }));
   EXPECT_CALL(*callMock, callback(RELAYS_STATUS_MATCH(RELAY_STATUS{RELAY_BATHROOM_LED, RELAY_STATE_ON})));
   EXPECT_EQ(RETURN_OK, rel_set(RELAY_BATHROOM_LED, RELAY_STATE_ON));
   EXPECT_EQ(RELAY_STATE_ON, rel_get(RELAY_BATHROOM_AC));
   EXPECT_EQ(RELAY_STATE_ON, rel_get(RELAY_BATHROOM_LED));

   /**
    * <b>scenario</b>: Set relay - setting RELAY_BATHROOM_AC to OFF <br>
    * <b>expected</b>: Relay 13 set to OFF, previous relay unchanged <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,_,_)).WillOnce(Invoke(
         [&](I2C_ADDRESS addr, const uint8_t* data, uint8_t size) -> I2C_STATUS
         {
            EXPECT_EQ(addr, 0x10);
            EXPECT_EQ(0xFFFD, *((uint16_t*)data));
            EXPECT_EQ(size, 2U);
            return I2C_STATUS_OK;
         }));
   EXPECT_CALL(*callMock, callback(RELAYS_STATUS_MATCH(RELAY_STATUS{RELAY_BATHROOM_AC, RELAY_STATE_OFF})));
   EXPECT_EQ(RETURN_OK, rel_set(RELAY_BATHROOM_AC, RELAY_STATE_OFF));
   EXPECT_EQ(RELAY_STATE_OFF, rel_get(RELAY_BATHROOM_AC));
   EXPECT_EQ(RELAY_STATE_ON, rel_get(RELAY_BATHROOM_LED));
   /**
    * <b>scenario</b>: Set relay - setting RELAY_BATHROOM_LED to OFF <br>
    * <b>expected</b>: Relay 2 set to OFF, previous relay unchanged <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,_,_)).WillOnce(Invoke(
         [&](I2C_ADDRESS addr, const uint8_t* data, uint8_t size) -> I2C_STATUS
         {
            EXPECT_EQ(addr, 0x10);
            EXPECT_EQ(0xFFFF, *((uint16_t*)data));
            EXPECT_EQ(size, 2U);
            return I2C_STATUS_OK;
         }));
   EXPECT_CALL(*callMock, callback(RELAYS_STATUS_MATCH(RELAY_STATUS{RELAY_BATHROOM_LED, RELAY_STATE_OFF})));
   EXPECT_EQ(RETURN_OK, rel_set(RELAY_BATHROOM_LED, RELAY_STATE_OFF));
   EXPECT_EQ(RELAY_STATE_OFF, rel_get(RELAY_BATHROOM_AC));
   EXPECT_EQ(RELAY_STATE_OFF, rel_get(RELAY_BATHROOM_LED));

   EXPECT_EQ(RETURN_OK, rel_remove_listener(&fake_callback));
}

/**
 * @test Getting state of all relays.
 */
TEST_F(relaysBoardFixture, relay_get_all_tests)
{
   /**
    * <b>scenario</b>: RELAY_BATHROOM_AC is ON, getting state of all relays <br>
    * <b>expected</b>: Correct state returned <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_write(_,_,_)).WillOnce(Invoke(
         [&](I2C_ADDRESS addr, const uint8_t* data, uint8_t size) -> I2C_STATUS
         {
            EXPECT_EQ(addr, 0x10);
            EXPECT_EQ(0xEFFF, *((uint16_t*)data));
            EXPECT_EQ(size, 2U);
            return I2C_STATUS_OK;
         }));
   EXPECT_EQ(RETURN_OK, rel_set(RELAY_BATHROOM_AC, RELAY_STATE_ON));

   RELAY_STATUS result [16];
   EXPECT_EQ(RETURN_OK, rel_get_all(result));

   EXPECT_EQ(result[0].id, RELAY_WARDROBE_LED);
   EXPECT_EQ(result[0].state, RELAY_STATE_OFF);

   EXPECT_EQ(result[5].id, RELAY_KITCHEN_WALL);
   EXPECT_EQ(result[5].state, RELAY_STATE_OFF);

   EXPECT_EQ(result[7].id, RELAY_BATHROOM_AC);
   EXPECT_EQ(result[7].state, RELAY_STATE_ON);

   EXPECT_EQ(result[10].id, RELAY_WARDROBE_AC);
   EXPECT_EQ(result[10].state, RELAY_STATE_OFF);

}

/**
 * @test Autoupdate mechanism tests.
 */
TEST_F(relaysBoardFixture, relay_autoupdate_tests)
{
   /**
    * <b>scenario</b>: Simulate autoupdate request, no data change <br>
    * <b>expected</b>: Data from I2C read <br>
    * ************************************************
    */
   EXPECT_EQ(rel_module.current_relays, 0x0000);
   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&](I2C_ADDRESS addr , uint8_t* buf, uint8_t size) -> I2C_STATUS
         {
            EXPECT_EQ(size, 2);
            EXPECT_EQ(cfg.address + 1, addr);
            buf[0] = 0xFF;
            buf[1] = 0xFF;
            return I2C_STATUS_OK;
         }));
   rel_read_state();

   EXPECT_EQ(rel_module.current_relays, 0x0000);

   /**
    * <b>scenario</b>: Simulate autoupdate request, i2c error <br>
    * <b>expected</b>: Data from I2C read, relays state not updated <br>
    * ************************************************
    */
   EXPECT_EQ(rel_module.current_relays, 0x0000);
   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&](I2C_ADDRESS addr , uint8_t* buf, uint8_t size) -> I2C_STATUS
         {
            EXPECT_EQ(size, 2);
            EXPECT_EQ(cfg.address + 1, addr);
            buf[0] = 0xFE;
            buf[1] = 0xFF;
            return I2C_STATUS_ERROR;
         }));
   rel_read_state();

   /**
    * <b>scenario</b>: Simulate autoupdate request, data changed <br>
    * <b>expected</b>: Data from I2C read, relays state updated <br>
    * ************************************************
    */
   EXPECT_EQ(rel_module.current_relays, 0x0000);
   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&](I2C_ADDRESS addr , uint8_t* buf, uint8_t size) -> I2C_STATUS
         {
            EXPECT_EQ(size, 2);
            EXPECT_EQ(cfg.address + 1, addr);
            buf[0] = 0xFE;
            buf[1] = 0xFF;
            return I2C_STATUS_OK;
         }));
   rel_read_state();

   EXPECT_EQ(rel_module.current_relays, 0x0001);

}

/**
 * @test Autoupdate mechanism period changes tests.
 */
TEST_F(relaysBoardFixture, relay_autoupdate_period_change_tests)
{
   /**
    * <b>scenario</b>: Setting too low period <br>
    * <b>expected</b>: Period not changed <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, rel_set_verification_period(RELAYS_VERIFICATION_MIN_TIME_MS - 1));
   EXPECT_EQ(rel_get_verification_period(), RELAYS_VERIFICATION_DEF_TIME_MS);

   /**
    * <b>scenario</b>: Setting too high period <br>
    * <b>expected</b>: Period not changed <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, rel_set_verification_period(RELAYS_VERIFICATION_MAX_TIME_MS + 1));
   EXPECT_EQ(rel_get_verification_period(), RELAYS_VERIFICATION_DEF_TIME_MS);

   /**
    * <b>scenario</b>: Setting correct period, but scheduler returns NOK <br>
    * <b>expected</b>: Period not changed  <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, _)).WillOnce(Return(RETURN_NOK));
   EXPECT_EQ(RETURN_NOK, rel_set_verification_period(RELAYS_VERIFICATION_DEF_TIME_MS + 1));
   EXPECT_EQ(rel_get_verification_period(), RELAYS_VERIFICATION_DEF_TIME_MS);


   /**
    * <b>scenario</b>: Setting correct period <br>
    * <b>expected</b>: Period changed, scheduler notified <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_period(_, _)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_OK, rel_set_verification_period(RELAYS_VERIFICATION_DEF_TIME_MS + 1));
   EXPECT_EQ(rel_get_verification_period(), RELAYS_VERIFICATION_DEF_TIME_MS + 1);

}

/**
 * @test Autoupdate mechanism ON/OFF tests.
 */
TEST_F(relaysBoardFixture, relay_autoupdate_on_off_tests)
{
   /**
    * <b>scenario</b>: Disabling autoupdate mechanism <br>
    * <b>expected</b>: Disabled correctly <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_,TASKSTATE_STOPPED));
   rel_disable_verification();
   EXPECT_EQ(rel_get_verification_state(), RETURN_NOK);

   /**
    * <b>scenario</b>: Enabling autoupdate mechanism <br>
    * <b>expected</b>: Enabled correctly <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_,TASKSTATE_RUNNING));
   rel_enable_verification();
   EXPECT_EQ(rel_get_verification_state(), RETURN_OK);

}

/**
 * @test Reading all relays state directly from board.
 */
TEST_F(relaysBoardFixture, relay_read_from_board_tests)
{
   RELAY_STATUS results [RELAYS_BOARD_COUNT];
   /**
    * <b>scenario</b>: No buffer provided <br>
    * <b>expected</b>: RETURN_NOK returned <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, rel_read_all(nullptr));

   /**
    * <b>scenario</b>: Cannot read I2C data <br>
    * <b>expected</b>: RETURN_NOK returned <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Return(I2C_STATUS_ERROR));
   EXPECT_EQ(RETURN_NOK, rel_read_all(results));

   /**
    * <b>scenario</b>: Relays read correctly <br>
    * <b>expected</b>: RETURN_OK returned <br>
    * ************************************************
    */
   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&](I2C_ADDRESS addr, uint8_t* buf, uint8_t size) -> I2C_STATUS
         {
            EXPECT_EQ(addr, cfg.address + 1);
            EXPECT_EQ(size, 2);
            buf[0] = 0xFE;
            buf[1] = 0xFF;
            return I2C_STATUS_OK;
         }));
   EXPECT_EQ(RETURN_OK, rel_read_all(results));

   EXPECT_EQ(results[0].id, RELAY_WARDROBE_LED);
   EXPECT_EQ(results[0].state, RELAY_STATE_ON);
   EXPECT_EQ(results[1].id, RELAY_BATHROOM_LED);
   EXPECT_EQ(results[1].state, RELAY_STATE_OFF);

}
