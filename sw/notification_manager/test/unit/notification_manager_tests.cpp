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
#include "time_counter_mock.h"
#include "wifimanager_mock.h"
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
      mock_time_counter_init();
      mock_wifimgr_init();
      mock_logger_init();
   }

   virtual void TearDown()
   {
      mock_env_deinit();
      mock_inp_deinit();
      mock_rel_deinit();
      mock_fan_deinit();
      mock_slm_deinit();
      mock_time_counter_deinit();
      mock_wifimgr_deinit();
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
   mock_wifimgr_init();
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
   EXPECT_CALL(*wifimgr_mock, wifimgr_register_data_callback(_)).WillOnce(Return(RETURN_OK));

   EXPECT_EQ(RETURN_OK, ntfmgr_init());

   mock_env_deinit();
   mock_inp_deinit();
   mock_rel_deinit();
   mock_fan_deinit();
   mock_slm_deinit();
   mock_wifimgr_deinit();

}

/**
 * @test This test case covers sending periodic notifications on particular module event
 */
TEST_F(ntfmgrFixture, periodic_notification_tests)
{
   /**
    * <b>scenario</b>: Event from Relays module received.<br>
    * <b>expected</b>: Correct NTF message sent.<br>
    * ************************************************
    */
   {
      RELAY_STATUS status = {RELAY_BEDROOM_AC, RELAY_STATE_ON};
      uint8_t expected_result [] = {NTF_GROUP_RELAYS, NTF_RELAYS_STATE, NTF_NTF, NTF_REPLY_UNKNOWN, 2, RELAY_BEDROOM_AC, RELAY_STATE_ON, '\n'};
      EXPECT_CALL(*wifimgr_mock, wifimgr_broadcast_bytes(_,_)).WillOnce(Invoke([&](const uint8_t* data, uint16_t size) -> RET_CODE
            {
               EXPECT_EQ(size, (sizeof(expected_result)/sizeof(expected_result[0])));
               for (uint8_t i = 0; i < size; i++)
               {
                  EXPECT_EQ(data[i], expected_result[i]);
               }
               return RETURN_OK;
            }));
      ntfmgr_on_relays_change(&status);
   }
   /**
    * <b>scenario</b>: Event from Inputs module received.<br>
    * <b>expected</b>: Correct NTF message sent.<br>
    * ************************************************
    */
   {
      INPUT_STATUS status = {INPUT_BEDROOM_AC, INPUT_STATE_ACTIVE};
      uint8_t expected_result [] = {NTF_GROUP_INPUTS, NTF_INPUTS_STATE, NTF_NTF, NTF_REPLY_UNKNOWN, 2, INPUT_BEDROOM_AC, INPUT_STATE_ACTIVE, '\n'};
      EXPECT_CALL(*wifimgr_mock, wifimgr_broadcast_bytes(_,_)).WillOnce(Invoke([&](const uint8_t* data, uint16_t size) -> RET_CODE
            {
               EXPECT_EQ(size, (sizeof(expected_result)/sizeof(expected_result[0])));
               for (uint8_t i = 0; i < size; i++)
               {
                  EXPECT_EQ(data[i], expected_result[i]);
               }
               return RETURN_OK;
            }));
      ntfmgr_on_inputs_change(status);
   }
   /**
    * <b>scenario</b>: Event from ENV module received.<br>
    * <b>expected</b>: Correct NTF message sent.<br>
    * ************************************************
    */
   {
      DHT_SENSOR sensor = {DHT_SENSOR1, DHT_TYPE_DHT22, {22, 5, 60, 1}};
      uint8_t expected_result [] = {NTF_GROUP_ENV, NTF_ENV_SENSOR_DATA, NTF_NTF, NTF_REPLY_UNKNOWN, 6, ENV_KITCHEN, DHT_TYPE_DHT22, 60, 1, 22, 5, '\n'};
      EXPECT_CALL(*wifimgr_mock, wifimgr_broadcast_bytes(_,_)).WillOnce(Invoke([&](const uint8_t* data, uint16_t size) -> RET_CODE
            {
               EXPECT_EQ(size, (sizeof(expected_result)/sizeof(expected_result[0])));
               for (uint8_t i = 0; i < size; i++)
               {
                  EXPECT_EQ(data[i], expected_result[i]);
               }
               return RETURN_OK;
            }));
      ntfmgr_on_env_change(ENV_EV_NEW_DATA, ENV_KITCHEN, &sensor);
   }
   /**
    * <b>scenario</b>: Event from FAN module received.<br>
    * <b>expected</b>: Correct NTF message sent.<br>
    * ************************************************
    */
   {
      uint8_t expected_result [] = {NTF_GROUP_FAN, NTF_FAN_STATE, NTF_NTF, NTF_REPLY_UNKNOWN, 1, FAN_STATE_SUSPEND, '\n'};
      EXPECT_CALL(*wifimgr_mock, wifimgr_broadcast_bytes(_,_)).WillOnce(Invoke([&](const uint8_t* data, uint16_t size) -> RET_CODE
            {
               EXPECT_EQ(size, (sizeof(expected_result)/sizeof(expected_result[0])));
               for (uint8_t i = 0; i < size; i++)
               {
                  EXPECT_EQ(data[i], expected_result[i]);
               }
               return RETURN_OK;
            }));
      ntfmgr_on_fan_change(FAN_STATE_SUSPEND);
   }
   /**
    * <b>scenario</b>: Event from SLM module received.<br>
    * <b>expected</b>: Correct NTF message sent.<br>
    * ************************************************
    */
   {
      uint8_t expected_result [] = {NTF_GROUP_SLM, NTF_SLM_STATE, NTF_NTF, NTF_REPLY_UNKNOWN, 1, SLM_STATE_ONGOING_OFF, '\n'};
      EXPECT_CALL(*wifimgr_mock, wifimgr_broadcast_bytes(_,_)).WillOnce(Invoke([&](const uint8_t* data, uint16_t size) -> RET_CODE
            {
               EXPECT_EQ(size, (sizeof(expected_result)/sizeof(expected_result[0])));
               for (uint8_t i = 0; i < size; i++)
               {
                  EXPECT_EQ(data[i], expected_result[i]);
               }
               return RETURN_OK;
            }));
      ntfmgr_on_slm_change(SLM_STATE_ONGOING_OFF);
   }
}

/**
 * @test This test case covers commands related to System module
 */
TEST_F(ntfmgrFixture, system_module_commands)
{
   /**
    * <b>scenario</b>: Set SYSTEM_TIME command received.<br>
    * <b>expected</b>: Correct NTF message sent.<br>
    * ************************************************
    */
   {
      uint8_t command [] = {NTF_GROUP_SYSTEM, NTF_SYSTEM_TIME, NTF_SET, NTF_REPLY_UNKNOWN, 7, 0x0F, 0x0B, 0x07, 0xE5, 10, 11, 12};
      uint8_t expected_result [] = {NTF_GROUP_SYSTEM, NTF_SYSTEM_TIME, NTF_SET, NTF_REPLY_OK, 0, '\n'};
      EXPECT_CALL(*wifimgr_mock, wifimgr_send_bytes(1, _,_)).WillOnce(Invoke([&](ServerClientID, const uint8_t* data, uint16_t size) -> RET_CODE
            {
               EXPECT_EQ(size, (sizeof(expected_result)/sizeof(expected_result[0])));
               for (uint8_t i = 0; i < size; i++)
               {
                  EXPECT_EQ(data[i], expected_result[i]);
               }
               return RETURN_OK;
            }));
      EXPECT_CALL(*time_cnt_mock, time_set_utc(_)).WillOnce(Invoke([&](TimeItem* time) -> RET_CODE
            {
               EXPECT_EQ(time->day, 15);
               EXPECT_EQ(time->month, 11);
               EXPECT_EQ(time->year, 2021);
               EXPECT_EQ(time->hour, 10);
               EXPECT_EQ(time->minute, 11);
               EXPECT_EQ(time->second, 12);
               return RETURN_OK;
            }));
      ntfmgr_parse_request(1, (char*) command);
   }

   /**
    * <b>scenario</b>: Set SYSTEM_TIME command received, but cannot set.<br>
    * <b>expected</b>: Correct NTF message sent.<br>
    * ************************************************
    */
   {
      uint8_t command [] = {NTF_GROUP_SYSTEM, NTF_SYSTEM_TIME, NTF_SET, NTF_REPLY_UNKNOWN, 7, 0x0F, 0x0B, 0x07, 0xE5, 10, 11, 12};
      uint8_t expected_result [] = {NTF_GROUP_SYSTEM, NTF_SYSTEM_TIME, NTF_SET, NTF_REPLY_NOK, 0, '\n'};
      EXPECT_CALL(*wifimgr_mock, wifimgr_send_bytes(1, _,_)).WillOnce(Invoke([&](ServerClientID, const uint8_t* data, uint16_t size) -> RET_CODE
            {
               EXPECT_EQ(size, (sizeof(expected_result)/sizeof(expected_result[0])));
               for (uint8_t i = 0; i < size; i++)
               {
                  EXPECT_EQ(data[i], expected_result[i]);
               }
               return RETURN_OK;
            }));
      EXPECT_CALL(*time_cnt_mock, time_set_utc(_)).WillOnce(Return(RETURN_NOK));
      ntfmgr_parse_request(1, (char*) command);
   }

   /**
    * <b>scenario</b>: Get SYSTEM_TIME command received, but cannot to set time<br>
    * <b>expected</b>: Correct NTF message sent.<br>
    * ************************************************
    */
   {
      uint8_t command [] = {NTF_GROUP_SYSTEM, NTF_SYSTEM_TIME, NTF_GET, NTF_REPLY_UNKNOWN, 0};
      uint8_t expected_result [] = {NTF_GROUP_SYSTEM, NTF_SYSTEM_TIME, NTF_GET, NTF_REPLY_NOK, 0, '\n'};
      EXPECT_CALL(*wifimgr_mock, wifimgr_send_bytes(1, _,_)).WillOnce(Invoke([&](ServerClientID, const uint8_t* data, uint16_t size) -> RET_CODE
            {
               EXPECT_EQ(size, (sizeof(expected_result)/sizeof(expected_result[0])));
               for (uint8_t i = 0; i < size; i++)
               {
                  EXPECT_EQ(data[i], expected_result[i]);
               }
               return RETURN_OK;
            }));

      EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(nullptr));
      ntfmgr_parse_request(1, (char*) command);
   }

   /**
    * <b>scenario</b>: Get SYSTEM_TIME command received.<br>
    * <b>expected</b>: Correct NTF message sent.<br>
    * ************************************************
    */
   {
      uint8_t command [] = {NTF_GROUP_SYSTEM, NTF_SYSTEM_TIME, NTF_GET, NTF_REPLY_UNKNOWN, 0};
      uint8_t expected_result [] = {NTF_GROUP_SYSTEM, NTF_SYSTEM_TIME, NTF_GET, NTF_REPLY_OK, 7, 0x0F, 0x0B, 0x07, 0xE5, 10, 11, 12, '\n'};
      EXPECT_CALL(*wifimgr_mock, wifimgr_send_bytes(1, _,_)).WillOnce(Invoke([&](ServerClientID, const uint8_t* data, uint16_t size) -> RET_CODE
            {
               EXPECT_EQ(size, (sizeof(expected_result)/sizeof(expected_result[0])));
               for (uint8_t i = 0; i < size; i++)
               {
                  EXPECT_EQ(data[i], expected_result[i]);
               }
               return RETURN_OK;
            }));

      TimeItem item = {15, 11, 2021, 10, 11, 12};
      EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&item));
      ntfmgr_parse_request(1, (char*) command);
   }
}
