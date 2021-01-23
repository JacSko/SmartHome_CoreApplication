#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/notification_manager.c"
#ifdef __cplusplus
}
#endif

#include "inputs_board_mock.h"
#include "relays_board_mock.h"
#include "bathroom_fan_mock.h"
#include "stairs_led_module_mock.h"
#include "env_monitor_mock.h"
#include "logger_mock.h"

/* ============================= */
/**
 * @file notification_manager_tests.cpp
 *
 * @brief Unit tests of Notification manager module
 *
 * @details
 * This tests verifies behavior of Notification manager module
 *
 * @author Jacek Skowronek
 * @date 23/01/2021
 */
/* ============================= */


using namespace ::testing;

struct ntfmgrFixture : public ::testing::Test
{
   virtual void SetUp()
   {
      mock_env_init();
      mock_inp_init();
      mock_rel_init();
      mock_fan_init();
      mock_slm_init();
      mock_logger_init();
   }

   virtual void TearDown()
   {
      mock_env_deinit();
      mock_inp_deinit();
      mock_rel_deinit();
      mock_fan_deinit();
      mock_slm_deinit();
      mock_logger_deinit();
   }
};

/**
 * @test Initialization of Notification manager module
 */
TEST(ntf_mgr, initialization)
{
   mock_env_init();
   mock_inp_init();
   mock_rel_init();
   mock_fan_init();
   mock_slm_init();
   mock_logger_init();

   /**
    * <b>scenario</b>: Module initialization.<br>
    * <b>expected</b>: Module registers callbacks in all modules.<br>
    * ************************************************
    */
   EXPECT_CALL(*inp_mock, inp_add_input_listener(_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*env_mock, env_register_listener(_, _)).Times(ENV_SENSORS_COUNT).WillRepeatedly(Return(RETURN_OK));
   EXPECT_CALL(*rel_mock, rel_add_listener(_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*fan_mock, fan_add_listener(_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*slm_mock, slm_add_listener(_)).WillOnce(Return(RETURN_OK));


   EXPECT_EQ(RETURN_OK, ntfmgr_init());

   mock_env_deinit();
   mock_inp_deinit();
   mock_rel_deinit();
   mock_fan_deinit();
   mock_slm_deinit();}
