#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/command_parser.c"
#ifdef __cplusplus
}
#endif

#include "wifimanager_mock.h"
#include "inputs_board_mock.h"
#include "relays_board_mock.h"
#include "i2c_driver_mock.h"
#include "dht_driver_mock.h"
#include "bathroom_fan_mock.h"
#include "env_monitor_mock.h"
#include "logger_mock.h"
#include "stairs_led_module_mock.h"
/* ============================= */
/**
 * @file command_parser_tests.cpp
 *
 * @brief Unit tests of Command Parser module
 *
 * @details
 * This test verify behavior of Command Parser module
 *
 * @author Jacek Skowronek
 * @date 11/12/2020
 */
/* ============================= */

using namespace ::testing;

struct callbackMock
{
	MOCK_METHOD1(bt_send_function, RET_CODE(const char*));
	MOCK_METHOD2(wifi_send_function, RET_CODE(ServerClientID id, const char*));
};

callbackMock* callMock;

RET_CODE bt_send_function(const char* data)
{
	return callMock->bt_send_function(data);
}

RET_CODE wifi_send_function(ServerClientID id, const char* data)
{
	return callMock->wifi_send_function(id, data);
}


struct cmdParserFixture : public ::testing::Test
{
	const uint16_t  LOGGER_BUFFER_SIZE = 512;
	virtual void SetUp()
	{
		mock_wifimgr_init();
		mock_inp_init();
		mock_rel_init();
		mock_i2c_init();
		mock_dht_init();
		mock_fan_init();
		mock_env_init();
		mock_logger_init();
		mock_slm_init();
		callMock = new callbackMock;

		cmd_register_bt_sender(&bt_send_function);
		cmd_register_wifi_sender(&wifi_send_function);
	}

	virtual void TearDown()
	{
		mock_wifimgr_deinit();
		mock_inp_deinit();
		mock_rel_deinit();
		mock_i2c_deinit();
		mock_dht_deinit();
		mock_fan_deinit();
		mock_env_deinit();
		mock_logger_deinit();
		mock_slm_deinit();
		delete callMock;
		cmd_unregister_bt_sender();
		cmd_unregister_wifi_sender();
	}
};

/**
 * @test Checks if module correctly receives, parse data and sends response
 */
TEST_F(cmdParserFixture, command_responding)
{
	const ServerClientID id = 1;
	/**
	 * <b>scenario</b>: Command from WiFi received.<br>
	 * <b>expected</b>: Echo sent, response sent.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*callMock, wifi_send_function(_,_))
			.WillOnce(Invoke([&](ServerClientID cliendid, const char* data) -> RET_CODE
			{
				EXPECT_EQ(id, cliendid);
				EXPECT_STREQ(data, "CMD: unknown command\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](ServerClientID cliendid, const char* data) -> RET_CODE
			{
				EXPECT_EQ(id, cliendid);
				EXPECT_STREQ(data, "ERROR\n");
				return RETURN_OK;
			}));
	cmd_handle_wifi_data(id, "unknown command");

	/**
	 * <b>scenario</b>: Command from BT received.<br>
	 * <b>expected</b>: Echo sent, response sent.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*callMock, bt_send_function(_))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "CMD: unknown command\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "ERROR\n");
				return RETURN_OK;
			}));
	cmd_handle_bt_data("unknown command");
}

/**
 * @test Checks WiFi Manager related command behavior
 */
TEST_F(cmdParserFixture, wifi_manager_cmds_test)
{
	/**
	 * <b>scenario</b>: WiFi manager reset command.<br>
	 * <b>expected</b>: Command executed, response sent.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*callMock, bt_send_function(_))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "CMD: wifimgr reset\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "OK\n");
				return RETURN_OK;
			}));
	EXPECT_CALL(*wifimgr_mock, wifimgr_reset()).WillOnce(Return(RETURN_OK));
	cmd_handle_bt_data("wifimgr reset");
	/**
	 * <b>scenario</b>: WiFi manager IP set command.<br>
	 * <b>expected</b>: Command executed, response sent.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*callMock, bt_send_function(_))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "CMD: wifimgr ip set 192.168.1.10\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "OK\n");
				return RETURN_OK;
			}));
	EXPECT_CALL(*wifimgr_mock, wifimgr_set_ip_address(_)).WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ("192.168.1.10", data);
				return RETURN_OK;
			}));
	cmd_handle_bt_data("wifimgr ip set 192.168.1.10");

	/**
	 * <b>scenario</b>: WiFi manager IP get command.<br>
	 * <b>expected</b>: Command executed, response sent.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*callMock, bt_send_function(_))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "CMD: wifimgr ip get\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "IP:111.222.111.222\n");
				return RETURN_OK;
			})).WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "OK\n");
				return RETURN_OK;
			}));
	EXPECT_CALL(*wifimgr_mock, wifimgr_get_ip_address(_)).WillOnce(Invoke([&](IPAddress* ip) -> RET_CODE
			{
				ip->ip_address[0] = 111;
				ip->ip_address[1] = 222;
				ip->ip_address[2] = 111;
				ip->ip_address[3] = 222;
				return RETURN_OK;
			}));
	cmd_handle_bt_data("wifimgr ip get");

	/**
	 * <b>scenario</b>: WiFi manager network set command.<br>
	 * <b>expected</b>: Command executed, response sent.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*callMock, bt_send_function(_))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "CMD: wifimgr network set NEW_SSID NEW_PASS\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "OK\n");
				return RETURN_OK;
			}));
	EXPECT_CALL(*wifimgr_mock, wifimgr_set_network_data(_,_)).WillOnce(Invoke([&](const char* ssid, const char* pass) -> RET_CODE
			{
				EXPECT_STREQ(ssid, "NEW_SSID");
				EXPECT_STREQ(pass, "NEW_PASS");
				return RETURN_OK;
			}));
	cmd_handle_bt_data("wifimgr network set NEW_SSID NEW_PASS");

	/**
	 * <b>scenario</b>: WiFi manager network get command.<br>
	 * <b>expected</b>: Command executed, response sent.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*callMock, bt_send_function(_))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "CMD: wifimgr network get\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "SSID:MY_SSID\n");
				return RETURN_OK;
			})).WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "OK\n");
				return RETURN_OK;
			}));
	EXPECT_CALL(*wifimgr_mock, wifimgr_get_network_name(_,_)).WillOnce(Invoke([&](char* data, uint8_t size) -> RET_CODE
			{
				string_format(data, "%s", "MY_SSID");
				return RETURN_OK;
			}));
	cmd_handle_bt_data("wifimgr network get");

	/**
	 * <b>scenario</b>: WiFi manager NTP server set.<br>
	 * <b>expected</b>: Command executed, response sent.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*callMock, bt_send_function(_))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "CMD: wifimgr ntpserver set www.test.pl\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "OK\n");
				return RETURN_OK;
			}));
	EXPECT_CALL(*wifimgr_mock, wifimgr_set_ntp_server(_)).WillOnce(Invoke([&](const char* ntp) -> RET_CODE
			{
				EXPECT_STREQ(ntp, "www.test.pl");
				return RETURN_OK;
			}));
	cmd_handle_bt_data("wifimgr ntpserver set www.test.pl");

	/**
	 * <b>scenario</b>: WiFi manager NTP server get.<br>
	 * <b>expected</b>: Command executed, response sent.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*callMock, bt_send_function(_))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "CMD: wifimgr ntpserver get\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "NTP:www.test.pl\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "OK\n");
				return RETURN_OK;
			}));
	EXPECT_CALL(*wifimgr_mock, wifimgr_get_ntp_server()).WillOnce(Invoke([&]() -> const char*
			{
				return "www.test.pl";
			}));
	cmd_handle_bt_data("wifimgr ntpserver get");

	/**
	 * <b>scenario</b>: WiFi manager server port set.<br>
	 * <b>expected</b>: Command executed, response sent.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*callMock, bt_send_function(_))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "CMD: wifimgr serverport set 1111\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "OK\n");
				return RETURN_OK;
			}));
	EXPECT_CALL(*wifimgr_mock, wifimgr_set_server_port(_)).WillOnce(Invoke([&](uint16_t port) -> RET_CODE
			{
				EXPECT_EQ(port, 1111);
				return RETURN_OK;
			}));
	cmd_handle_bt_data("wifimgr serverport set 1111");

	/**
	 * <b>scenario</b>: WiFi manager server port get.<br>
	 * <b>expected</b>: Command executed, response sent.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*callMock, bt_send_function(_))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "CMD: wifimgr serverport get\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "PORT:1111\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "OK\n");
				return RETURN_OK;
			}));
	EXPECT_CALL(*wifimgr_mock, wifimgr_get_server_port()).WillOnce(Return(1111));
	cmd_handle_bt_data("wifimgr serverport get");

	/**
	 * <b>scenario</b>: WiFi manager get clients details.<br>
	 * <b>expected</b>: Command executed, response sent.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*callMock, bt_send_function(_))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "CMD: wifimgr clients get\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "CLIENT1: type 1, 1.2.3.4\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
			EXPECT_STREQ(data, "CLIENT2: type 1, 5.6.7.8\n");
				return RETURN_OK;
			}))
			.WillOnce(Invoke([&](const char* data) -> RET_CODE
			{
				EXPECT_STREQ(data, "OK\n");
				return RETURN_OK;
			}));
	EXPECT_CALL(*wifimgr_mock, wifi_get_max_clients()).WillOnce(Return(2));
	EXPECT_CALL(*wifimgr_mock, wifimgr_get_clients_details(_)).WillOnce(Invoke([&](ClientID* buf) -> uint8_t
			{
				buf[0].id = 1;
				buf[0].type = TCP;
				buf[0].address.ip_address[0] = 1;
				buf[0].address.ip_address[1] = 2;
				buf[0].address.ip_address[2] = 3;
				buf[0].address.ip_address[3] = 4;
				buf[1].id = 2;
				buf[1].type = TCP;
				buf[1].address.ip_address[0] = 5;
				buf[1].address.ip_address[1] = 6;
				buf[1].address.ip_address[2] = 7;
				buf[1].address.ip_address[3] = 8;
				return 2;
			}));
	cmd_handle_bt_data("wifimgr clients get");
}

/**
 * @test Checks inputs board related command behavior
 */
TEST_F(cmdParserFixture, inputs_cmds_test)
{
   /**
    * <b>scenario</b>: Get all inputs command received.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: inp get_all\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "INP_STATE: 1:OFF 2:ON 3:OFF 4:ON 5:OFF 6:ON 7:OFF 8:ON 9:OFF 10:ON 11:OFF 12:ON 13:OFF 14:ON 15:OFF 16:ON\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*inp_mock, inp_get_all(_)).WillOnce(Invoke([&](INPUT_STATUS* status) -> RET_CODE
         {
            for (uint8_t i = 0; i < INPUTS_MAX_INPUT_LINES; i++)
            {
               status[i].state = i%2? INPUT_STATE_ACTIVE : INPUT_STATE_INACTIVE;
            }
            return RETURN_OK;
         }));
   cmd_handle_bt_data("inp get_all");

   /**
    * <b>scenario</b>: Read all inputs command received.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: inp read_all\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "INP_STATE: 1:OFF 2:ON 3:OFF 4:ON 5:OFF 6:ON 7:OFF 8:ON 9:OFF 10:ON 11:OFF 12:ON 13:OFF 14:ON 15:OFF 16:ON\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*inp_mock, inp_read_all(_)).WillOnce(Invoke([&](INPUT_STATUS* status) -> RET_CODE
         {
            for (uint8_t i = 0; i < INPUTS_MAX_INPUT_LINES; i++)
            {
               status[i].state = i%2? INPUT_STATE_ACTIVE : INPUT_STATE_INACTIVE;
            }
            return RETURN_OK;
         }));
   cmd_handle_bt_data("inp read_all");

   /**
    * <b>scenario</b>: Get config command received.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: inp get_config\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "INP_CONFIG:\naddress: 16\nitem:input no\n0:0\n1:1\n2:2\n3:3\n4:4\n5:5\n6:6\n7:7\n8:8\n9:9\n10:10\n11:11\n12:12\n13:13\n14:14\n15:15\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*inp_mock, inp_get_config(_)).WillOnce(Invoke([&](INPUTS_CONFIG* cfg) -> RET_CODE
         {
            cfg->address = 0x10;
            for (uint8_t i = 0; i < INPUTS_MAX_INPUT_LINES; i++)
            {
               cfg->items[i].input_no = i;
               cfg->items[i].item = (INPUT_ID)i;
            }
            return RETURN_OK;
         }));
   cmd_handle_bt_data("inp get_config");

   /**
    * <b>scenario</b>: Disable/Enable interrupt command received.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: inp enable_interrupt\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: inp disable_interrupt\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*inp_mock, inp_enable_interrupt());
   cmd_handle_bt_data("inp enable_interrupt");

   EXPECT_CALL(*inp_mock, inp_disable_interrupt());
   cmd_handle_bt_data("inp disable_interrupt");

   /**
    * <b>scenario</b>: Set debounce time command received.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: inp set_debounce_time 2000\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));

   EXPECT_CALL(*inp_mock, inp_set_debounce_time(2000)).WillOnce(Return(RETURN_OK));
   cmd_handle_bt_data("inp set_debounce_time 2000");

   /**
    * <b>scenario</b>: Get debounce time command received.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: inp get_debounce_time\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "DEB_TIME:1000\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));

   EXPECT_CALL(*inp_mock, inp_get_debounce_time()).WillOnce(Return(1000));
   cmd_handle_bt_data("inp get_debounce_time");

   /**
    * <b>scenario</b>: Set update time command received.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: inp set_update_time 2000\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));

   EXPECT_CALL(*inp_mock, inp_set_periodic_update_time(2000)).WillOnce(Return(RETURN_OK));
   cmd_handle_bt_data("inp set_update_time 2000");

   /**
    * <b>scenario</b>: Get update time command received.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: inp get_update_time\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "UPD_TIME:1000\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));

   EXPECT_CALL(*inp_mock, inp_get_periodic_update_time()).WillOnce(Return(1000));
   cmd_handle_bt_data("inp get_update_time");


   /**
    * <b>scenario</b>: Enable/Disable periodic update.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: inp enable_update\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: inp disable_update\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*inp_mock, inp_enable_periodic_update());
   cmd_handle_bt_data("inp enable_update");

   EXPECT_CALL(*inp_mock, inp_disable_periodic_update());
   cmd_handle_bt_data("inp disable_update");

}

/**
 * @test Checks relays board related command behavior
 */
TEST_F(cmdParserFixture, relays_cmds_test)
{
   /**
    * <b>scenario</b>: Get all relays command received.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: rel get_all\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "REL_STATE:1:OFF 2:ON 3:OFF 4:ON 5:OFF 6:ON 7:OFF 8:ON 9:OFF 10:ON 11:OFF 12:ON 13:OFF 14:ON 15:OFF 16:ON\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*rel_mock, rel_get_all(_)).WillOnce(Invoke([&](RELAY_STATUS* status) -> RET_CODE
         {
            for (uint8_t i = 0; i < RELAYS_BOARD_COUNT; i++)
            {
               status[i].state = i%2? RELAY_STATE_ON : RELAY_STATE_OFF;
            }
            return RETURN_OK;
         }));
   cmd_handle_bt_data("rel get_all");

   /**
    * <b>scenario</b>: Read all relayss command received.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: rel read_all\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "REL_STATE:1:OFF 2:ON 3:OFF 4:ON 5:OFF 6:ON 7:OFF 8:ON 9:OFF 10:ON 11:OFF 12:ON 13:OFF 14:ON 15:OFF 16:ON\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*rel_mock, rel_read_all(_)).WillOnce(Invoke([&](RELAY_STATUS* status) -> RET_CODE
         {
            for (uint8_t i = 0; i < RELAYS_BOARD_COUNT; i++)
            {
               status[i].state = i%2? RELAY_STATE_ON : RELAY_STATE_OFF;
            }
            return RETURN_OK;
         }));
   cmd_handle_bt_data("rel read_all");

   /**
    * <b>scenario</b>: Get config command received.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: rel get_config\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "REL_CONFIG:\naddress: 16\nitem:relay no\n0:0\n1:1\n2:2\n3:3\n4:4\n5:5\n6:6\n7:7\n8:8\n9:9\n10:10\n11:11\n12:12\n13:13\n14:14\n15:15\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*rel_mock, rel_get_config(_)).WillOnce(Invoke([&](RELAYS_CONFIG* cfg) -> RET_CODE
         {
            cfg->address = 0x10;
            for (uint8_t i = 0; i < INPUTS_MAX_INPUT_LINES; i++)
            {
               cfg->items[i].relay_no = i;
               cfg->items[i].id = (RELAY_ID)i;
            }
            return RETURN_OK;
         }));
   cmd_handle_bt_data("rel get_config");

   /**
    * <b>scenario</b>: Set update time command received.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: rel set_update_time 2000\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));

   EXPECT_CALL(*rel_mock, rel_set_verification_period(2000)).WillOnce(Return(RETURN_OK));
   cmd_handle_bt_data("rel set_update_time 2000");

   /**
    * <b>scenario</b>: Get update time command received.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: rel get_update_time\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "UPD_TIME:1000\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));

   EXPECT_CALL(*rel_mock, rel_get_verification_period()).WillOnce(Return(1000));
   cmd_handle_bt_data("rel get_update_time");


   /**
    * <b>scenario</b>: Enable/Disable periodic update.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: rel enable_update\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: rel disable_update\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*rel_mock, rel_enable_verification());
   cmd_handle_bt_data("rel enable_update");

   EXPECT_CALL(*rel_mock, rel_disable_verification());
   cmd_handle_bt_data("rel disable_update");

}

/**
 * @test Checks i2c driver related command behavior
 */
TEST_F(cmdParserFixture, i2cdrv_cmds_test)
{
   /**
    * <b>scenario</b>: Get timeout.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: i2c get_timeout\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "TIMEOUT:100\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*i2c_mock, i2c_get_timeout()).WillOnce(Return(100));
   cmd_handle_bt_data("i2c get_timeout");

   /**
    * <b>scenario</b>: Set timeout.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: i2c set_timeout 200\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*i2c_mock, i2c_set_timeout(200)).WillOnce(Return(RETURN_OK));
   cmd_handle_bt_data("i2c set_timeout 200");

   /**
    * <b>scenario</b>: Reset command received.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: i2c reset\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*i2c_mock, i2c_reset());
   cmd_handle_bt_data("i2c reset");

   /**
    * <b>scenario</b>: Write data.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: i2c write_data 10 2 255 253\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*i2c_mock, i2c_write(_,_,_)).WillOnce(Invoke([&](I2C_ADDRESS addr, const uint8_t* data, uint8_t size)
         {
            EXPECT_EQ(addr, 10);
            EXPECT_EQ(data[0], 255);
            EXPECT_EQ(data[1], 253);
            EXPECT_EQ(size, 2);
            return I2C_STATUS_OK;
         }));
   cmd_handle_bt_data("i2c write_data 10 2 255 253");

   /**
    * <b>scenario</b>: Read data.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: i2c read_data 10 2\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "DATA: 255 253\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*i2c_mock, i2c_read(_,_,_)).WillOnce(Invoke([&](I2C_ADDRESS addr, uint8_t* data, uint8_t size)
         {
            EXPECT_EQ(addr, 10);
            data[0] = 255;
            data[1] = 253;
            EXPECT_EQ(size, 2);
            return I2C_STATUS_OK;
         }));
   cmd_handle_bt_data("i2c read_data 10 2");
}

/**
 * @test Checks dht driver related command behavior
 */
TEST_F(cmdParserFixture, dhtdrv_cmds_test)
{
   /**
    * <b>scenario</b>: Get timeout.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: dht get_timeout\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "TIMEOUT:100\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*dht_mock, dht_get_timeout()).WillOnce(Return(100));
   cmd_handle_bt_data("dht get_timeout");

   /**
    * <b>scenario</b>: Set timeout.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: dht set_timeout 200\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*dht_mock, dht_set_timeout(200)).WillOnce(Return(RETURN_OK));
   cmd_handle_bt_data("dht set_timeout 200");

   /**
    * <b>scenario</b>: Sensor read.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: dht read_sensor 1\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "DATA:23.2C 60.1% t:1\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*dht_mock, dht_read(_,_)).WillOnce(Invoke([&](DHT_SENSOR_ID id, DHT_SENSOR* sensor) -> DHT_STATUS
         {
            EXPECT_EQ(id, DHT_SENSOR2);
            sensor->data.temp_h = 23;
            sensor->data.temp_l = 2;
            sensor->data.hum_h = 60;
            sensor->data.hum_l = 1;
            sensor->type = DHT_TYPE_DHT22;
            return DHT_STATUS_OK;
         }));
   cmd_handle_bt_data("dht read_sensor 1");
}

/**
 * @test Checks bathroom fan related command behavior
 */
TEST_F(cmdParserFixture, bathfan_cmds_test)
{
   /**
    * <b>scenario</b>: Fan start.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: fan start\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*fan_mock, fan_start()).WillOnce(Return(RETURN_OK));
   cmd_handle_bt_data("fan start");

   /**
    * <b>scenario</b>: Fan stop.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: fan stop\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*fan_mock, fan_stop()).WillOnce(Return(RETURN_OK));
   cmd_handle_bt_data("fan stop");

   /**
    * <b>scenario</b>: Fan set working time.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: fan set_max_work_time 1000\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: fan set_min_work_time 100\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*fan_mock, fan_set_max_working_time(1000)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*fan_mock, fan_set_min_working_time(100)).WillOnce(Return(RETURN_OK));
   cmd_handle_bt_data("fan set_max_work_time 1000");
   cmd_handle_bt_data("fan set_min_work_time 100");

   /**
    * <b>scenario</b>: Fan set humidity thresholds.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: fan set_hum_thr 80\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: fan set_thr_hyst 10\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*fan_mock, fan_set_humidity_threshold(80)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*fan_mock, fan_set_threshold_hysteresis(10)).WillOnce(Return(RETURN_OK));
   cmd_handle_bt_data("fan set_hum_thr 80");
   cmd_handle_bt_data("fan set_thr_hyst 10");

   /**
    * <b>scenario</b>: Get state.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: fan get_state\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "STATE:1\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));;
   EXPECT_CALL(*fan_mock, fan_get_state()).WillOnce(Return(FAN_STATE_ON));
   cmd_handle_bt_data("fan get_state");

   /**
    * <b>scenario</b>: Get config.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: fan get_config\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "FAN_CONFIG:\nMIN_WORK_TIME:10\nMAX_WORK_TIME:100\nHUM_THR:80\nTHR_HYST:5\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*fan_mock, fan_get_config(_)).WillOnce(Invoke([&](FAN_CONFIG* cfg) ->RET_CODE
         {
            cfg->fan_humidity_threshold = 80;
            cfg->fan_threshold_hysteresis = 5;
            cfg->max_working_time_s = 100;
            cfg->min_working_time_s = 10;
            return RETURN_OK;
         }));
   cmd_handle_bt_data("fan get_config");

}

/**
 * @test Checks env module related command behavior
 */
TEST_F(cmdParserFixture, env_cmds_test)
{
   /**
    * <b>scenario</b>: Read sensor.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: env read_sensor 1\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "DATA:22.1C 60.1% t:0\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*env_mock, env_read_sensor(ENV_OUTSIDE,_)).WillOnce(Invoke([&](ENV_ITEM_ID, DHT_SENSOR* sensor) -> RET_CODE
         {
            sensor->data.temp_h = 22;
            sensor->data.temp_l = 1;
            sensor->data.hum_h = 60;
            sensor->data.hum_l = 1;
            sensor->type = DHT_TYPE_DHT11;
            return RETURN_OK;
         }));
   cmd_handle_bt_data("env read_sensor 1");

   /**
    * <b>scenario</b>: Read sensor error rates.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: env read_error 1\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "DATA:\nNR:20% CS:10%\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));

   ENV_ERROR_RATE result;
   result.cs_err_rate = 10;
   result.nr_err_rate = 20;
   EXPECT_CALL(*env_mock, env_get_error_stats(ENV_OUTSIDE)).WillOnce(Return(result));
   cmd_handle_bt_data("env read_error 1");

   /**
    * <b>scenario</b>: Set measurement period.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: env set_meas_period 2000\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*env_mock, env_set_measurement_period(2000)).WillOnce(Return(RETURN_OK));
   cmd_handle_bt_data("env set_meas_period 2000");

   /**
    * <b>scenario</b>: Get measurement period.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: env get_meas_period\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "PERIOD:2000\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*env_mock, env_get_measurement_period()).WillOnce(Return(2000));
   cmd_handle_bt_data("env get_meas_period");

}

/**
 * @test Checks logger related command behavior
 */
TEST_F(cmdParserFixture, log_cmds_test)
{
   /**
    * <b>scenario</b>: Enable/disable module.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: log enable module\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: log disable module\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*logger_mock, logger_enable()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*logger_mock, logger_disable());
   cmd_handle_bt_data("log enable module");
   cmd_handle_bt_data("log disable module");

   /**
    * <b>scenario</b>: Enable/disable group.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: log enable DEBUG\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: log disable DEBUG\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*logger_mock, logger_set_group_state(LOG_DEBUG, LOGGER_GROUP_ENABLE)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*logger_mock, logger_set_group_state(LOG_DEBUG, LOGGER_GROUP_DISABLE)).WillOnce(Return(RETURN_OK));
   cmd_handle_bt_data("log enable DEBUG");
   cmd_handle_bt_data("log disable DEBUG");

   /**
    * <b>scenario</b>: Get groups state.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: log groups_state\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            /* string with groups state */
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*logger_mock, logger_get_group_state(_)).WillRepeatedly(Return(LOGGER_GROUP_ENABLE));
   cmd_handle_bt_data("log groups_state");

}

/**
 * @test Checks SLM related command behavior
 */
TEST_F(cmdParserFixture, slm_cmds_test)
{
   /**
    * <b>scenario</b>: Get config.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: slm get_config\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "SLM_CONFIG:\nID:0\nOFF_MODE:1\naddr:5\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*slm_mock, slm_get_config(_)).WillOnce(Invoke([]
           (SLM_CONFIG* cfg) -> RET_CODE
           {
               cfg->address = 0x05;
               cfg->off_effect_mode = SLM_OFF_EFFECT_ENABLED;
               cfg->program_id = SLM_PROGRAM1;
               return RETURN_OK;
           }));
   cmd_handle_bt_data("slm get_config");

   /**
    * <b>scenario</b>: Start/stop program.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: slm start\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: slm start_alwon\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: slm stop\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*slm_mock, slm_start_program()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*slm_mock, slm_start_program_alw_on()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*slm_mock, slm_stop_program()).WillOnce(Return(RETURN_OK));
   cmd_handle_bt_data("slm start");
   cmd_handle_bt_data("slm start_alwon");
   cmd_handle_bt_data("slm stop");

   /**
    * <b>scenario</b>: Get status.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: slm status\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "STATUS:state:1, id:2\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*slm_mock, slm_get_state()).WillOnce(Return(SLM_STATE_ONGOING_ON));
   EXPECT_CALL(*slm_mock, slm_get_current_program_id()).WillOnce(Return(SLM_PROGRAM3));
   cmd_handle_bt_data("slm status");

   /**
    * <b>scenario</b>: Set program.<br>
    * <b>expected</b>: Command executed, response sent.<br>
    * ************************************************
    */
   EXPECT_CALL(*callMock, bt_send_function(_))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "CMD: slm set_program 1\n");
            return RETURN_OK;
         }))
         .WillOnce(Invoke([&](const char* data) -> RET_CODE
         {
            EXPECT_STREQ(data, "OK\n");
            return RETURN_OK;
         }));
   EXPECT_CALL(*slm_mock, slm_set_current_program_id(SLM_PROGRAM2));
   cmd_handle_bt_data("slm set_program 1");
}
