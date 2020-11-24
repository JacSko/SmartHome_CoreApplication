#include "gtest/gtest.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/wifi_driver.c"
#ifdef __cplusplus
}
#endif

#include "uartengine_mock.h"
#include "time_counter_mock.h"
/**
 * @brief Unit test of wifi driver.
 *
 * All tests that verify behavior of WiFi driver module
 *
 * @file wifi_driver_tests.cpp
 * @author  Jacek Skowronek
 * @date    21/11/2020
 */


using namespace ::testing;

struct callbackMock
{
	MOCK_METHOD1(callback, void(const char*));
};

callbackMock* callMock;

void fake_callback(const char* buf)
{
	callMock->callback(buf);
}

struct wifiFixture : public ::testing::Test
{
	virtual void SetUp()
	{
		mock_time_counter_init();
		mock_uartengine_init();
		callMock = new callbackMock();
	}

	virtual void TearDown()
	{
		mock_time_counter_deinit();
		mock_uartengine_deinit();
		delete callMock;
	}
};

/**
 * @test WiFi initialization
 */
TEST_F(wifiFixture, engine_initialization)
{
	/**
	 * @<b>scenario<\b>: Module initialization - cannot initialize uart.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_CALL(*uartengineMock, uartengine_initialize(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifi_initialize());

	/**
	 * @<b>scenario<\b>: Module initialization - cannot register UART callback.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_CALL(*uartengineMock, uartengine_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_register_callback(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifi_initialize());

	/**
	 * @<b>scenario<\b>: Module initialization - cannot send handshake command (OK).
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_CALL(*uartengineMock, uartengine_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_register_callback(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifi_initialize());

	/**
	 * @<b>scenario<\b>: Module initialization - timeout when waiting for response.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_CALL(*uartengineMock, uartengine_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_register_callback(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_NOK));
	TimeItem t1, t2 = {};
	t2.time_raw = t1.time_raw + 5000;	//5s means that timeout exceeded
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1))
										   .WillOnce(Return(&t2));
	EXPECT_EQ(RETURN_NOK, wifi_initialize());

	/**
	 * @<b>scenario<\b>: Module initialization - Wrong response received
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	const char response [] = "ERROR";
	EXPECT_CALL(*uartengineMock, uartengine_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_register_callback(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response));
	t1 = {};
	EXPECT_CALL(*time_cnt_mock, time_get()).WillRepeatedly(Return(&t1));
	EXPECT_EQ(RETURN_NOK, wifi_initialize());

	/**
	 * @<b>scenario<\b>: Module initialization - correct sequence
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	const char correct_response [] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_register_callback(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(correct_response));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillRepeatedly(Return(&t1));
	EXPECT_EQ(RETURN_OK, wifi_initialize());
}

/**
 * @test Data callback register/unregister
 */
TEST_F(wifiFixture, callback_register_unregister)
{
	/**
	 * @<b>scenario<\b>: Callback registering when there was no callback registered previously.
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	EXPECT_EQ(RETURN_OK, wifi_register_data_callback(&fake_callback));

	/**
	 * @<b>scenario<\b>: Callback registering when there was callback registered previously.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_EQ(RETURN_NOK, wifi_register_data_callback(&fake_callback));

	/**
	 * @<b>scenario<\b>: Callback unregistered and registered again.
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	wifi_unregister_data_callback();
	EXPECT_EQ(RETURN_OK, wifi_register_data_callback(&fake_callback));
}

/**
 * @test WiFi network connection
 */
TEST_F(wifiFixture, wifi_network_connect)
{
	/**
	 * @<b>scenario<\b>: SSID is null.
	 * @<b>expected<\b>: RETURN_ERROR returned.
	 */
	EXPECT_EQ(RETURN_ERROR, wifi_connect_to_network(NULL, ""));

	/**
	 * @<b>scenario<\b>: PASS is null.
	 * @<b>expected<\b>: RETURN_ERROR returned.
	 */
	EXPECT_EQ(RETURN_ERROR, wifi_connect_to_network("", NULL));

	/**
	 * @<b>scenario<\b>: Cannot send command to device.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifi_connect_to_network("SSID", "PASS"));

	/**
	 * @<b>scenario<\b>: Timeout when waiting for first response
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	TimeItem t1, t2 = {};
	t2.time_raw = t1.time_raw + 60000;

	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
																{
																	if (!data) return RETURN_NOK;
																	EXPECT_STREQ(data, "AT+CWJAP_CUR=\"SSID\",\"PASS\"\r\n");
																	return RETURN_OK;
																}));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1)).
											WillOnce(Return(&t2));
	EXPECT_EQ(RETURN_NOK, wifi_connect_to_network("SSID", "PASS"));

	/**
	 * @<b>scenario<\b>: Wrong first response received
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */

	const char wifi_conn_wrong[] = "WIFI_NOT_CONNECTED";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK)).
															   WillOnce(Return(RETURN_NOK));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_conn_wrong));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1)).
											WillOnce(Return(&t1)).
											WillOnce(Return(&t1)).
											WillOnce(Return(&t2));
	EXPECT_EQ(RETURN_NOK, wifi_connect_to_network("SSID", "PASS"));

	/**
	 * @<b>scenario<\b>: Wrong second response received
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	const char wifi_conn_corr[] = "WIFI CONNECTED";
	const char wifi_ip_got_wrong[] = "WIFI_GOT_NO_IP";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK)).
															   WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_conn_corr))
														 .WillOnce(Return(wifi_ip_got_wrong));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1)).
											WillOnce(Return(&t1)).
											WillOnce(Return(&t1)).
											WillOnce(Return(&t1)).
											WillOnce(Return(&t2));;
	EXPECT_EQ(RETURN_NOK, wifi_connect_to_network("SSID", "PASS"));

	/**
	 * @<b>scenario<\b>: Correct connection sequence
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	const char wifi_ip_got_ok[] = "WIFI GOT IP";
	const char wifi_conn_ok[] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK)).
															   WillOnce(Return(RETURN_OK)).
															   WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_conn_corr))
														 .WillOnce(Return(wifi_ip_got_ok))
														 .WillOnce(Return(wifi_conn_ok));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1)).
											WillOnce(Return(&t1)).
											WillOnce(Return(&t1)).
											WillOnce(Return(&t1)).
											WillOnce(Return(&t1)).
											WillOnce(Return(&t1));
	EXPECT_EQ(RETURN_OK, wifi_connect_to_network("SSID", "PASS"));
}

/**
 * @test WiFi network disconnection
 */
TEST_F(wifiFixture, wifi_network_disconnect)
{
	/**
	 * @<b>scenario<\b>: Disconnect request - wrong response received.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	TimeItem t1 = {};
	TimeItem t2 = {};
	t2.time_raw = t1.time_raw + 10000;
	const char wifi_disconn_error[] = "ERROR";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CWQAP\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t2));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillRepeatedly(Return(wifi_disconn_error));
	EXPECT_EQ(RETURN_NOK, wifi_disconnect_from_network());

	/**
	 * @<b>scenario<\b>: Disconnect request - correct response received.
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	const char wifi_disconn_ok[] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CWQAP\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1))
										   .WillOnce(Return(&t1));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_disconn_ok));
	EXPECT_EQ(RETURN_OK, wifi_disconnect_from_network());

}

/**
 * @test WiFi MAC address set
 */
TEST_F(wifiFixture, wifi_mac_address_set)
{
	/**
	 * @<b>scenario<\b>: Set MAC request - wrong response received.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	TimeItem t1 = {};
	TimeItem t2 = {};
	t2.time_raw = t1.time_raw + 10000;
	const char wifi_disconn_error[] = "ERROR";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSTAMAC_CUR=\"MAC\"\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t2));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillRepeatedly(Return(wifi_disconn_error));
	EXPECT_EQ(RETURN_NOK, wifi_set_mac_address("MAC"));

	/**
	 * @<b>scenario<\b>: Disconnect request - correct response received.
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	const char wifi_disconn_ok[] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSTAMAC_CUR=\"MAC\"\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1))
										   .WillOnce(Return(&t1));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_disconn_ok));
	EXPECT_EQ(RETURN_OK, wifi_set_mac_address("MAC"));

}

/**
 * @test WiFi set max server clients
 */
TEST_F(wifiFixture, wifi_max_server_clients)
{
	/**
	 * @<b>scenario<\b>: Set max server clients - wrong response received.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	TimeItem t1 = {};
	TimeItem t2 = {};
	t2.time_raw = t1.time_raw + 10000;
	const char wifi_disconn_error[] = "ERROR";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSERVERMAXCONN=5\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t2));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillRepeatedly(Return(wifi_disconn_error));
	EXPECT_EQ(RETURN_NOK, wifi_set_max_server_clients(5));

	/**
	 * @<b>scenario<\b>: Disconnect request - correct response received.
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	const char wifi_disconn_ok[] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSERVERMAXCONN=5\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1))
										   .WillOnce(Return(&t1));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_disconn_ok));
	EXPECT_EQ(RETURN_OK, wifi_set_max_server_clients(5));

}

/**
 * @test WiFi open udp server
 */
TEST_F(wifiFixture, wifi_open_udp_server)
{
	/**
	 * @<b>scenario<\b>: Open UDP server - wrong response received.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	TimeItem t1 = {};
	TimeItem t2 = {};
	t2.time_raw = t1.time_raw + 10000;
	const char wifi_disconn_error[] = "ERROR";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSERVER=1,2222\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t2));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillRepeatedly(Return(wifi_disconn_error));
	EXPECT_EQ(RETURN_NOK, wifi_open_udp_server(2222));

	/**
	 * @<b>scenario<\b>: Disconnect request - correct response received.
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	const char wifi_disconn_ok[] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSERVER=1,2222\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1))
										   .WillOnce(Return(&t1));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_disconn_ok));
	EXPECT_EQ(RETURN_OK, wifi_open_udp_server(2222));
}

/**
 * @test WiFi close udp server
 */
TEST_F(wifiFixture, wifi_close_udp_server)
{
	/**
	 * @<b>scenario<\b>: Open UDP server - wrong response received.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	TimeItem t1 = {};
	TimeItem t2 = {};
	t2.time_raw = t1.time_raw + 10000;
	const char wifi_disconn_error[] = "ERROR";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSERVER=0\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t1))
										   .WillOnce(Return(&t2));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillRepeatedly(Return(wifi_disconn_error));
	EXPECT_EQ(RETURN_NOK, wifi_close_udp_server());

	/**
	 * @<b>scenario<\b>: Disconnect request - correct response received.
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	const char wifi_disconn_ok[] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSERVER=0\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1))
										   .WillOnce(Return(&t1));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_disconn_ok));
	EXPECT_EQ(RETURN_OK, wifi_close_udp_server());

}
