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

/**
 * @brief Unit test of command parser.
 *
 * All tests that verify behavior of command parser module.
 *
 * @file command_parser_tests.cpp
 * @author  Jacek Skowronek
 * @date    12/12/2020
 */


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
		callMock = new callbackMock;

		cmd_register_bt_sender(&bt_send_function);
		cmd_register_wifi_sender(&wifi_send_function);
	}

	virtual void TearDown()
	{
		mock_wifimgr_deinit();
		delete callMock;
		cmd_unregister_bt_sender();
		cmd_unregister_wifi_sender();
	}
};

/**
 * @test Checks if module correctly receives data and sends response
 */
TEST_F(cmdParserFixture, command_responding)
{
	const ServerClientID id = 1;
	/**
	 * @<b>scenario<\b>: Command from WiFi received.
	 * @<b>expected<\b>: Echo sent, response sent.
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
	 * @<b>scenario<\b>: Command from BT received.
	 * @<b>expected<\b>: Echo sent, response sent.
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
	 * @<b>scenario<\b>: WiFi manager reset command.
	 * @<b>expected<\b>: Command executed, response sent.
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
	 * @<b>scenario<\b>: WiFi manager IP set command.
	 * @<b>expected<\b>: Command executed, response sent.
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
	 * @<b>scenario<\b>: WiFi manager IP get command.
	 * @<b>expected<\b>: Command executed, response sent.
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
	 * @<b>scenario<\b>: WiFi manager network set command.
	 * @<b>expected<\b>: Command executed, response sent.
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
	 * @<b>scenario<\b>: WiFi manager network get command.
	 * @<b>expected<\b>: Command executed, response sent.
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
	 * @<b>scenario<\b>: WiFi manager NTP server set.
	 * @<b>expected<\b>: Command executed, response sent.
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
	 * @<b>scenario<\b>: WiFi manager NTP server get.
	 * @<b>expected<\b>: Command executed, response sent.
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
	 * @<b>scenario<\b>: WiFi manager server port set.
	 * @<b>expected<\b>: Command executed, response sent.
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
	 * @<b>scenario<\b>: WiFi manager server port get.
	 * @<b>expected<\b>: Command executed, response sent.
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
	 * @<b>scenario<\b>: WiFi manager get clients details.
	 * @<b>expected<\b>: Command executed, response sent.
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






