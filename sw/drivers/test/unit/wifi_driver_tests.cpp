#include "gtest/gtest.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/wifi_driver.c"
#ifdef __cplusplus
}
#endif

#include "uartengine_mock.h"
#include "system_timestamp_mock.h"
#include "logger_mock.h"

/* ============================= */
/**
 * @file wifi_driver_tests.cpp
 *
 * @brief Unit tests of WiFi driver module
 *
 * @details
 * This test verify behavior of WiFi driver module
 *
 * @author Jacek Skowronek
 * @date 11/12/2020
 */
/* ============================= */

using namespace ::testing;

struct callbackMock
{
	MOCK_METHOD3(callback, void(ClientEvent, ServerClientID, const char*));
};

callbackMock* callMock;

void fake_callback(ClientEvent ev, ServerClientID id, const char* data)
{
	callMock->callback(ev, id, data);
}

struct wifiFixture : public ::testing::Test
{
	virtual void SetUp()
	{
		mock_ts_init();
		mock_logger_init();
		mock_uartengine_init();
		callMock = new callbackMock();
	}

	virtual void TearDown()
	{
	   mock_ts_deinit();
		mock_logger_deinit();
		mock_uartengine_deinit();
		delete callMock;
	}
	const WIFI_UART_Config uart_config = {115200, 1024, 512};
};

/**
 * @test WiFi initialization
 */
TEST_F(wifiFixture, engine_initialization)
{
	/**
	 * <b>scenario</b>: Module initialization - cannot initialize uart.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_initialize(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifi_initialize(&uart_config));
	Mock::VerifyAndClearExpectations(uartengineMock);

	/**
	 * <b>scenario</b>: Module initialization - cannot register UART callback.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_register_callback(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifi_initialize(&uart_config));
	Mock::VerifyAndClearExpectations(uartengineMock);

	/**
	 * <b>scenario</b>: Module initialization - cannot reset module.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_register_callback(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifi_initialize(&uart_config));
	Mock::VerifyAndClearExpectations(uartengineMock);

	/**
	 * <b>scenario</b>: Module initialization - cannot disable echo.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_register_callback(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK))
														   .WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*ts_mock, ts_wait(_)).Times(1);
	EXPECT_CALL(*uartengineMock, uartengine_clear_rx()).Times(1);
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_NOK));
	EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
	EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(5000));
	EXPECT_EQ(RETURN_NOK, wifi_initialize(&uart_config));
	Mock::VerifyAndClearExpectations(uartengineMock);


	/**
	 * <b>scenario</b>: Module initialization - cannot send test pattern.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	const char correct_response [] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_register_callback(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK))
														   .WillOnce(Return(RETURN_OK))
														   .WillOnce(Return(RETURN_NOK));
	EXPECT_CALL(*ts_mock, ts_wait(_)).Times(1);
	EXPECT_CALL(*uartengineMock, uartengine_clear_rx()).Times(1);
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(correct_response));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_EQ(RETURN_NOK, wifi_initialize(&uart_config));
	Mock::VerifyAndClearExpectations(uartengineMock);

	/**
	 * <b>scenario</b>: Module initialization - correct sequence<br>
	 * <b>expected</b>: RETURN_OK returned.<br>
	 * ************************************************
	 */

	EXPECT_CALL(*uartengineMock, uartengine_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_register_callback(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK))
														   .WillOnce(Return(RETURN_OK))
														   .WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*ts_mock, ts_wait(_)).Times(1);
	EXPECT_CALL(*uartengineMock, uartengine_clear_rx()).Times(1);
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(correct_response))
														 .WillOnce(Return(correct_response));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0))
                                  .WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillRepeatedly(Return(0));
	EXPECT_EQ(RETURN_OK, wifi_initialize(&uart_config));
	Mock::VerifyAndClearExpectations(uartengineMock);
}

/**
 * @test Data callback register/unregister
 */
TEST_F(wifiFixture, callback_register_unregister)
{
	/**
	 * <b>scenario</b>: Callback registering when there was no callback registered previously.<br>
	 * <b>expected</b>: RETURN_OK returned.<br>
	 * ************************************************
	 */
	EXPECT_EQ(RETURN_OK, wifi_register_client_event_callback(&fake_callback));

	/**
	 * <b>scenario</b>: Callback registering when there was callback registered previously.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_EQ(RETURN_NOK, wifi_register_client_event_callback(&fake_callback));

	/**
	 * <b>scenario</b>: Callback unregistered and registered again.<br>
	 * <b>expected</b>: RETURN_OK returned.<br>
	 * ************************************************
	 */
	wifi_unregister_client_event_callback();
	EXPECT_EQ(RETURN_OK, wifi_register_client_event_callback(&fake_callback));

	/**
	 * <b>scenario</b>: Call attempt when callback not registered.<br>
	 * <b>expected</b>: Callback not registered.<br>
	 * ************************************************
	 */
	wifi_unregister_client_event_callback();
	EXPECT_CALL(*callMock, callback(_,_,_)).Times(0);
	wifi_call_client_callback({},{}, NULL);
}

/**
 * @test WiFi network connection
 */
TEST_F(wifiFixture, wifi_network_connect)
{
	/**
	 * <b>scenario</b>: SSID is null.<br>
	 * <b>expected</b>: RETURN_ERROR returned.<br>
	 * ************************************************
	 */
	EXPECT_EQ(RETURN_ERROR, wifi_connect_to_network(NULL, ""));

	/**
	 * <b>scenario</b>: PASS is null.<br>
	 * <b>expected</b>: RETURN_ERROR returned.<br>
	 * ************************************************
	 */
	EXPECT_EQ(RETURN_ERROR, wifi_connect_to_network("", NULL));

	/**
	 * <b>scenario</b>: Cannot send command to device.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifi_connect_to_network("SSID", "PASS"));

	/**
	 * <b>scenario</b>: Timeout when waiting for first response<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
																{
																	if (!data) return RETURN_NOK;
																	EXPECT_STREQ(data, "AT+CWJAP_CUR=\"SSID\",\"PASS\"\r\n");
																	return RETURN_OK;
																}));
	EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
	EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(60000));
	EXPECT_EQ(RETURN_NOK, wifi_connect_to_network("SSID", "PASS"));

	/**
	 * <b>scenario</b>: Wrong first response received<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */

	const char wifi_conn_wrong[] = "WIFI_NOT_CONNECTED";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK)).
															   WillOnce(Return(RETURN_NOK));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_conn_wrong));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
                                        .WillOnce(Return(0))
                                        .WillOnce(Return(60000));
	EXPECT_EQ(RETURN_NOK, wifi_connect_to_network("SSID", "PASS"));

	/**
	 * <b>scenario</b>: Wrong second response received<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	const char wifi_conn_corr[] = "WIFI CONNECTED";
	const char wifi_ip_got_wrong[] = "WIFI_GOT_NO_IP";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK)).
															   WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_conn_corr))
														 .WillOnce(Return(wifi_ip_got_wrong));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0))
                                  .WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
                                        .WillOnce(Return(0))
                                        .WillOnce(Return(60000));
	EXPECT_EQ(RETURN_NOK, wifi_connect_to_network("SSID", "PASS"));

	/**
	 * <b>scenario</b>: OK response not received.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	const char wifi_ip_got[] = "WIFI GOT IP";
	const char wifi_error[] = "ERROR";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_conn_corr))
														 .WillOnce(Return(wifi_ip_got))
														 .WillOnce(Return(wifi_error));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0))
                                  .WillOnce(Return(0))
                                  .WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
                                        .WillOnce(Return(0))
                                        .WillOnce(Return(0))
                                        .WillOnce(Return(60000));
	EXPECT_EQ(RETURN_NOK, wifi_connect_to_network("SSID", "PASS"));

	/**
	 * <b>scenario</b>: Correct connection sequence.<br>
	 * <b>expected</b>: RETURN_OK returned.<br>
	 * ************************************************
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
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0))
                                  .WillOnce(Return(0))
                                  .WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
                                        .WillOnce(Return(0))
                                        .WillOnce(Return(0));
	EXPECT_EQ(RETURN_OK, wifi_connect_to_network("SSID", "PASS"));
}

/**
 * @test WiFi network disconnection
 */
TEST_F(wifiFixture, wifi_network_disconnect)
{
	/**
	 * <b>scenario</b>: Disconnect request - wrong response received.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	const char wifi_disconn_error[] = "ERROR";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CWQAP\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
                                        .WillOnce(Return(0))
                                        .WillOnce(Return(60000));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillRepeatedly(Return(wifi_disconn_error));
	EXPECT_EQ(RETURN_NOK, wifi_disconnect_from_network());

	/**
	 * <b>scenario</b>: Disconnect request - correct response received.<br>
	 * <b>expected</b>: RETURN_OK returned.<br>
	 * ************************************************
	 */
	const char wifi_disconn_ok[] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CWQAP\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_disconn_ok));
	EXPECT_EQ(RETURN_OK, wifi_disconnect_from_network());

}

/**
 * @test WiFi MAC address set
 */
TEST_F(wifiFixture, wifi_mac_address_set)
{
	/**
	 * <b>scenario</b>: Set MAC request - string is NULL.<br>
	 * <b>expected</b>: RETURN_ERROR returned.<br>
	 * ************************************************
	 */
	EXPECT_EQ(RETURN_ERROR, wifi_set_mac_address(NULL));
	/**
	 * <b>scenario</b>: Set MAC request - wrong response received.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	const char wifi_disconn_error[] = "ERROR";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSTAMAC_CUR=\"MAC\"\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
	EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
                                        .WillOnce(Return(10000));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillRepeatedly(Return(wifi_disconn_error));
	EXPECT_EQ(RETURN_NOK, wifi_set_mac_address("MAC"));

	/**
	 * <b>scenario</b>: Disconnect request - correct response received.<br>
	 * <b>expected</b>: RETURN_OK returned.<br>
	 * ************************************************
	 */
	const char wifi_disconn_ok[] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSTAMAC_CUR=\"MAC\"\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_disconn_ok));
	EXPECT_EQ(RETURN_OK, wifi_set_mac_address("MAC"));

}

/**
 * @test WiFi open udp server
 */
TEST_F(wifiFixture, wifi_open_server)
{
	/**
	 * <b>scenario</b>: Open UDP server - wrong response received.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	const char wifi_disconn_error[] = "ERROR";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSERVER=1,2222\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(10000));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillRepeatedly(Return(wifi_disconn_error));
	EXPECT_EQ(RETURN_NOK, wifi_open_server(2222));

	/**
	 * <b>scenario</b>: Disconnect request - correct response received.<br>
	 * <b>expected</b>: RETURN_OK returned.<br>
	 * ************************************************
	 */
	const char wifi_disconn_ok[] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSERVER=1,2222\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_disconn_ok));
	EXPECT_EQ(RETURN_OK, wifi_open_server(2222));
}

/**
 * @test WiFi close udp server
 */
TEST_F(wifiFixture, wifi_close_server)
{
	/**
	 * <b>scenario</b>: Open UDP server - wrong response received.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	const char wifi_disconn_error[] = "ERROR";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSERVER=0\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(10000));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillRepeatedly(Return(wifi_disconn_error));
	EXPECT_EQ(RETURN_NOK, wifi_close_server());

	/**
	 * <b>scenario</b>: Disconnect request - correct response received.<br>
	 * <b>expected</b>: RETURN_OK returned.<br>
	 * ************************************************
	 */
	const char wifi_disconn_ok[] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSERVER=0\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillRepeatedly(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wifi_disconn_ok));
	EXPECT_EQ(RETURN_OK, wifi_close_server());

}

/**
 * @test WiFi UART data receiving
 */
TEST_F(wifiFixture, wifi_uart_data_receive_test)
{
	EXPECT_EQ(RETURN_OK, wifi_register_client_event_callback(fake_callback));

	/**
	 * <b>scenario</b>: New client connected - wrong string received.<br>
	 * <b>expected</b>: No callbacks called.<br>
	 * ************************************************
	 */

	EXPECT_CALL(*callMock, callback(_,_,_)).Times(0);
	wifi_on_uart_data("1,CONN");
	Mock::VerifyAndClearExpectations(callMock);

	/**
	 * <b>scenario</b>: New client connected - correct string received.<br>
	 * <b>expected</b>: Correct callback called.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*callMock, callback(_,_,_)).WillOnce(Invoke([&](ClientEvent ev, ServerClientID id, const char* data)
			{
				EXPECT_EQ(ev, CLIENT_CONNECTED);
				EXPECT_EQ(id, 1U);
				EXPECT_TRUE(data == NULL);
			}));
	std::string connect_ok = "1,CONNECT";
	wifi_on_uart_data(connect_ok.c_str());
	Mock::VerifyAndClearExpectations(callMock);

	/**
	 * <b>scenario</b>: Client disconnected - correct string received.<br>
	 * <b>expected</b>: Correct callback called.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*callMock, callback(_,_,_)).WillOnce(Invoke([&](ClientEvent ev, ServerClientID id, const char* data)
			{
				EXPECT_EQ(ev, CLIENT_DISCONNECTED);
				EXPECT_EQ(id, 1U);
				EXPECT_TRUE(data == NULL);
			}));
	std::string disconnect_ok = "1,CLOSED";
	wifi_on_uart_data(disconnect_ok.c_str());
	Mock::VerifyAndClearExpectations(callMock);

	/**
	 * <b>scenario</b>: New data from client.<br>
	 * <b>expected</b>: Correct callback called.<br>
	 * ************************************************
	 */
	std::string client_data = "+IPD,1,30:TEST_STRING_RECEIVED_FROM_CLIENT";
	EXPECT_CALL(*callMock, callback(_,_,_)).WillOnce(Invoke([&](ClientEvent ev, ServerClientID id, const char* data)
			{
				EXPECT_EQ(ev, CLIENT_DATA);
				EXPECT_EQ(id, 1U);
				EXPECT_STREQ(data, "TEST_STRING_RECEIVED_FROM_CLIENT");
			}));
	wifi_on_uart_data(client_data.c_str());
	Mock::VerifyAndClearExpectations(callMock);
}

/**
 * @test WiFi multiple connection on/off
 */
TEST_F(wifiFixture, wifi_multiple_connection_test)
{
	/**
	 * <b>scenario</b>: Enable multiple connections.<br>
	 * <b>expected</b>: Correct data sent.<br>
	 * ************************************************
	 */
	const char response_ok [] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPMUX=1\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response_ok));
	EXPECT_EQ(RETURN_OK, wifi_allow_multiple_clients(1));

	/**
	 * <b>scenario</b>: Enable multiple connections.<br>
	 * <b>expected</b>: Correct data sent.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPMUX=0\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response_ok));
	EXPECT_EQ(RETURN_OK, wifi_allow_multiple_clients(0));
}

/**
 * @test WiFi sending data to client
 */
TEST_F(wifiFixture, wifi_send_data_to_client_test)
{
	const char wrong_char []= "NOK";
	const char correct_char []= "OK";
	const char wrong_response [] = "SEND NOK";
	const char correct_response [] = "SEND OK";
	/**
	 * <b>scenario</b>: Send data to client - sign '>' not received.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
									{
										if (!data) return RETURN_NOK;
										EXPECT_STREQ(data, "AT+CIPSEND=1,12\r\n");
										return RETURN_OK;
									}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK));

   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(5000));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wrong_char))
  														 .WillOnce(Return(wrong_char))
														 .WillOnce(Return(wrong_char));

	EXPECT_EQ(RETURN_NOK, wifi_send_data(1, "TEST_STRING\n", 12));
	Mock::VerifyAndClearExpectations(uartengineMock);

	/* <b>scenario</b>: Send data to client - wrong response received.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
									{
										if (!data) return RETURN_NOK;
										EXPECT_STREQ(data, "AT+CIPSEND=1,12\r\n");
										return RETURN_OK;
									})).WillOnce(Invoke([&](const char * data)->RET_CODE
									{
										if (!data) return RETURN_NOK;
										EXPECT_STREQ(data, "TEST_STRING\n");
										return RETURN_OK;
									}));

	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK));

   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0))
      .WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(5000));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(correct_char))
														 .WillOnce(Return(wrong_response))
														 .WillOnce(Return(wrong_response))
														 .WillOnce(Return(wrong_response));


	EXPECT_EQ(RETURN_NOK, wifi_send_data(1, "TEST_STRING\n", 12));

	Mock::VerifyAndClearExpectations(uartengineMock);

	/* <b>scenario</b>: Send data to client - wrong response received.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
									{
										if (!data) return RETURN_NOK;
										EXPECT_STREQ(data, "AT+CIPSEND=1,12\r\n");
										return RETURN_OK;
									})).WillOnce(Invoke([&](const char * data)->RET_CODE
									{
										if (!data) return RETURN_NOK;
										EXPECT_STREQ(data, "TEST_STRING\n");
										return RETURN_OK;
									}));

	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK));

   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0))
      .WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(correct_char))
														 .WillOnce(Return(correct_response));

	EXPECT_EQ(RETURN_OK, wifi_send_data(1, "TEST_STRING\n", 12));
}

/**
 * @test WiFi connect / disconnect server
 */
TEST_F(wifiFixture, wifi_connect_disconnect_test)
{
	/**
	 * <b>scenario</b>: Connect to remote server via TCP successfully.<br>
	 * <b>expected</b>: RETURN_OK returned.<br>
	 * ************************************************
	 */
	const char response_ok [] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSTART=\"TCP\",\"www.test.pl\",123\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response_ok));
	EXPECT_EQ(RETURN_OK, wifi_connect_server(TCP, "www.test.pl", 123));

	/**
	 * <b>scenario</b>: Connect to remote server successfully.<br>
	 * <b>expected</b>: RETURN_OK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSTART=\"SSL\",\"www.test.pl\",123\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response_ok));
	EXPECT_EQ(RETURN_OK, wifi_connect_server(SSL, "www.test.pl", 123));

	/**
	 * <b>scenario</b>: Connect to remote server successfully.<br>
	 * <b>expected</b>: RETURN_OK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSTART=\"UDP\",\"www.test.pl\",123\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response_ok));
	EXPECT_EQ(RETURN_OK, wifi_connect_server(UDP, "www.test.pl", 123));

	/**
	 * <b>scenario</b>: Connection with remote server closing.<br>
	 * <b>expected</b>: Correct data sent.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPCLOSE\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response_ok));
	EXPECT_EQ(RETURN_OK, wifi_disconnect_server());
}

/**
 * @test WiFi setting IP address
 */
TEST_F(wifiFixture, wifi_set_ip_address)
{
	/**
	 * <b>scenario</b>: Set current IP address.<br>
	 * <b>expected</b>: RETURN_OK returned.<br>
	 * ************************************************
	 */
	IPAddress address = {};
	address.ip_address[0] = 192;
	address.ip_address[1] = 168;
	address.ip_address[2] = 100;
	address.ip_address[3] = 111;
	const char response_ok [] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSTA_CUR=\"192.168.100.111\"\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response_ok));
	EXPECT_EQ(RETURN_OK, wifi_set_ip_address(&address));
}

/**
 * @test WiFi get current IP address
 */
TEST_F(wifiFixture, wifi_get_current_ip_address)
{
	IPAddress address = {};

	/**
	 * <b>scenario</b>: IP string is NULL.<br>
	 * <b>expected</b>: RETURN_ERROR returned.<br>
	 * ************************************************
	 */
	EXPECT_EQ(RETURN_ERROR, wifi_get_ip_address(NULL));

	/**
	 * <b>scenario</b>: Cannot send command over UART<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifi_get_ip_address(&address));

	/**
	 * <b>scenario</b>: No response received from UART<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_NOK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(60000));
	EXPECT_EQ(RETURN_NOK, wifi_get_ip_address(&address));

	/**
	 * <b>scenario</b>: Wrong response from UART<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	const char wrong_response [] = "+CWADP:aaaa:bbbb:ccc";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wrong_response));
	EXPECT_EQ(RETURN_NOK, wifi_get_ip_address(&address));

	/**
	 * <b>scenario</b>: Get current IP address.<br>
	 * <b>expected</b>: Correct IP address returned.<br>
	 * ************************************************
	 */
	const char ip [] = "+CIPSTA_CUR:ip:\"192.168.1.1\"";
	const char gateway [] = "+CIPSTA_CUR:gateway:\"192.168.2.2\"";
	const char netmask [] = "+CIPSTA_CUR:netmask:\"192.168.3.3\"";


	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSTA_CUR?\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK))
	 														  .WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(ip))
														 .WillOnce(Return(gateway))
														 .WillOnce(Return(netmask));
	EXPECT_EQ(RETURN_OK, wifi_get_ip_address(&address));
	EXPECT_EQ(address.ip_address[0], 192);
	EXPECT_EQ(address.ip_address[1], 168);
	EXPECT_EQ(address.ip_address[2], 1);
	EXPECT_EQ(address.ip_address[3], 1);

	EXPECT_EQ(address.gateway[0], 192);
	EXPECT_EQ(address.gateway[1], 168);
	EXPECT_EQ(address.gateway[2], 2);
	EXPECT_EQ(address.gateway[3], 2);

	EXPECT_EQ(address.netmask[0], 192);
	EXPECT_EQ(address.netmask[1], 168);
	EXPECT_EQ(address.netmask[2], 3);
	EXPECT_EQ(address.netmask[3], 3);

}

/**
 * @test WiFi get current network name
 */
TEST_F(wifiFixture, wifi_get_current_network_name)
{
	char ssid [100];
	/**
	 * <b>scenario</b>: Get current network name - buffer is too small.<br>
	 * <b>expected</b>: Received data not wrote to buffer, RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_EQ(RETURN_ERROR, wifi_get_current_network_name(NULL, 10));

	/**
	 * <b>scenario</b>: Cannot send command over UART<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifi_get_current_network_name(ssid, 100));

	/**
	 * <b>scenario</b>: No response received from UART<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_NOK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(60000));
	EXPECT_EQ(RETURN_NOK, wifi_get_current_network_name(ssid, 100));

	/**
	 * <b>scenario</b>: Wrong response from UART<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	const char wrong_response [] = "+CWADP:aaaa:bbbb:ccc";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wrong_response));
	EXPECT_EQ(RETURN_NOK, wifi_get_current_network_name(ssid, 100));

	/**
	 * <b>scenario</b>: Get current network name - buffer is too small.<br>
	 * <b>expected</b>: Received data not wrote to buffer, RETURN_NOK returned.<br>
	 * ************************************************
	 */
	const char response [] = "+CWJAP_CUR:\"NIE_KRADNIJ_INTERNETU!!!\",\"f8:75:88:73:37:80\",1,-60";


	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CWJAP_CUR?\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response));
	EXPECT_EQ(RETURN_NOK, wifi_get_current_network_name(ssid, 10));

	/**
	 * <b>scenario</b>: Get current network name - buffer is too small.<br>
	 * <b>expected</b>: Received data not wrote to buffer, RETURN_NOK returned.<br>
	 * ************************************************
	 */
	const char response2 [] = "No AP";

	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CWJAP_CUR?\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response2));
	EXPECT_EQ(RETURN_NOK, wifi_get_current_network_name(ssid, 10));


	/**
	 * <b>scenario</b>: Get current network name.<br>
	 * <b>expected</b>: Correct network name returned.<br>
	 * ************************************************
	 */;
	const char response3 [] = "+CWJAP_CUR:\"NIE_KRADNIJ_INTERNETU!!!\",\"f8:75:88:73:37:80\",1,-60";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CWJAP_CUR?\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
   EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response3));
	EXPECT_EQ(RETURN_OK, wifi_get_current_network_name(ssid, 100));

}

/**
 * @test WiFi request Client details
 */
TEST_F(wifiFixture, wifi_request_client_details)
{
	ClientID id = {};
	/**
	 * <b>scenario</b>: Get client details- NULL argument<br>
	 * <b>expected</b>: RETURN_ERROR returned.<br>
	 * ************************************************
	 */
	EXPECT_EQ(RETURN_ERROR, wifi_request_client_details(NULL));

	/**
	 * <b>scenario</b>: Cannot send command over UART<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifi_request_client_details(&id));

	/**
	 * <b>scenario</b>: No response received from UART<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_NOK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(60000));
	EXPECT_EQ(RETURN_NOK, wifi_request_client_details(&id));

	/**
	 * <b>scenario</b>: Wrong response from UART<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	const char wrong_response [] = "+CWADP:aaaa:bbbb:ccc";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0))
      .WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(60000));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(wrong_response));
	EXPECT_EQ(RETURN_NOK, wifi_request_client_details(&id));

	/**
	 * <b>scenario</b>: Get client details - correct response received.<br>
	 * <b>expected</b>: Data parsed correctly, RETURN_OK returned.<br>
	 * ************************************************
	 */
	ClientID client = {};
	const char response1 [] = "+CIPSTATUS:0,\"TCP\",\"192.168.100.230\",43340,4444,1";
	const char response2 [] = "+CIPSTATUS:1,\"TCP\",\"192.168.100.233\",43340,4444,1";
	const char responseOK [] = "OK";

	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Invoke([&](const char * data)->RET_CODE
								{
									if (!data) return RETURN_NOK;
									EXPECT_STREQ(data, "AT+CIPSTATUS\r\n");
									return RETURN_OK;
								}));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response1))
														 .WillOnce(Return(response2))
														 .WillOnce(Return(responseOK));
	client.id = 1;
	EXPECT_EQ(RETURN_OK, wifi_request_client_details(&client));
	EXPECT_EQ(TCP, client.type);
	EXPECT_EQ(client.address.ip_address[0], 192);
	EXPECT_EQ(client.address.ip_address[1], 168);
	EXPECT_EQ(client.address.ip_address[2], 100);
	EXPECT_EQ(client.address.ip_address[3], 233);
}

/**
 * @test WiFi chip communication test
 */
TEST_F(wifiFixture, wifi_communication_test)
{
	/**
	 * <b>scenario</b>: Cannot send test command.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(wifi_test(), RETURN_NOK);

	/**
	 * <b>scenario</b>: No response received from chip.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_NOK))
															  .WillOnce(Return(RETURN_NOK))
															  .WillOnce(Return(RETURN_NOK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(10000));
	EXPECT_EQ(wifi_test(), RETURN_NOK);

	/**
	 * <b>scenario</b>: Correct sequence.<br>
	 * <b>expected</b>: RETURN_OK returned.<br>
	 * ************************************************
	 */
	const char response [] = "OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response));
	EXPECT_EQ(wifi_test(), RETURN_OK);

}

/**
 * @test WiFi NTP time reading
 */
TEST_F(wifiFixture, wifi_ntp_time_test)
{
	TimeItem test_item = {};

	/**
	 * <b>scenario</b>: NTP server is NULL.<br>
	 * <b>expected</b>: RETURN_ERROR returned.<br>
	 * ************************************************
	 */
	EXPECT_EQ(RETURN_ERROR, wifi_get_time(NULL, &test_item));

	/**
	 * <b>scenario</b>: Time item is NULL.<br>
	 * <b>expected</b>: RETURN_ERROR returned.<br>
	 * ************************************************
	 */
	EXPECT_EQ(RETURN_ERROR, wifi_get_time("www.server.pl", NULL));

	/**
	 * <b>scenario</b>: Cannot connect to server.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_NOK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(60000));
	EXPECT_EQ(wifi_get_time("www.server.pl", &test_item), RETURN_NOK);

	/**
	 * <b>scenario</b>: Cannot enable sending mode, server should be closed.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */

	const char response_ok[] = "OK";
	const char response_error[] = "ERROR";
	const char response_send_ok[] = "SEND OK";
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK))
														   .WillOnce(Return(RETURN_OK))
														   .WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(60000))
      .WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response_ok))
														 .WillOnce(Return(response_error))
														 .WillOnce(Return(response_ok));

	EXPECT_EQ(wifi_get_time("www.server.pl", &test_item), RETURN_NOK);

	/**
	 * <b>scenario</b>: NTP command send confirmation not received.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK)) //connect server
														   .WillOnce(Return(RETURN_OK)) //CIPSEND
														   .WillOnce(Return(RETURN_OK)) //request
														   .WillOnce(Return(RETURN_OK));//close server
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK)) //resp to server connec
															  .WillOnce(Return(RETURN_OK)) //resp to cipsend
															  .WillOnce(Return(RETURN_OK)) //wrong resp
															  .WillOnce(Return(RETURN_OK)); //server close

	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response_ok))
														 .WillOnce(Return(response_ok))
														 .WillOnce(Return(response_error))
														 .WillOnce(Return(response_ok));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(60000))
      .WillOnce(Return(0));
	EXPECT_EQ(wifi_get_time("www.server.pl", &test_item), RETURN_NOK);

	/**
	 * <b>scenario</b>: NTP response not received.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK))
														   .WillOnce(Return(RETURN_OK))
														   .WillOnce(Return(RETURN_OK))
														   .WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK));

	EXPECT_CALL(*uartengineMock, uartengine_count_bytes()).WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_clear_rx());
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(60000))
      .WillOnce(Return(0));
	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response_ok))
														 .WillOnce(Return(response_ok))
														 .WillOnce(Return(response_send_ok))
														 .WillOnce(Return(response_ok));

	EXPECT_EQ(wifi_get_time("www.server.pl", &test_item), RETURN_NOK);

	/**
	 * <b>scenario</b>: Correct NTP read out sequence.<br>
	 * <b>expected</b>: Correct time decoded, RETURN_OK returned.<br>
	 * ************************************************
	 */

	const uint8_t ntp_response [58] = {'\r', '\n', '+','I','P','D', ',', '4', '8', ':',
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0xE3, 0x57, 0x88, 0xC0, 0x00, 0x00, 0x00, 0x00};
	/* 1605175872 (unix) + 2208988800 (NTP offset) = 3814164672 */
	/* Time: 12-11-2020 10:11:12 UTC*/
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK))
														   .WillOnce(Return(RETURN_OK))
														   .WillOnce(Return(RETURN_OK))
														   .WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*uartengineMock, uartengine_can_read_string()).WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK))
															  .WillOnce(Return(RETURN_OK));

	EXPECT_CALL(*uartengineMock, uartengine_get_string()).WillOnce(Return(response_ok))
														 .WillOnce(Return(response_ok))
														 .WillOnce(Return(response_send_ok))
														 .WillOnce(Return(response_ok));
   EXPECT_CALL(*ts_mock, ts_get()).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0));
   EXPECT_CALL(*ts_mock, ts_get_diff(_)).WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0));
   EXPECT_CALL(*uartengineMock, uartengine_clear_rx());
	EXPECT_CALL(*uartengineMock, uartengine_count_bytes()).WillOnce(Return(58)); /* 48 bytes NTP and 8 bytes header */
	EXPECT_CALL(*uartengineMock, uartengine_get_bytes()).WillOnce(Return(ntp_response));
	EXPECT_EQ(wifi_get_time("www.server.pl", &test_item), RETURN_OK);
	EXPECT_EQ(test_item.year, 2020);
	EXPECT_EQ(test_item.month, 11);
	EXPECT_EQ(test_item.day, 12);
	EXPECT_EQ(test_item.hour, 10);
	EXPECT_EQ(test_item.minute, 11);
	EXPECT_EQ(test_item.second, 12);
}

/**
 * @test WiFi chip reset test
 */
TEST_F(wifiFixture, wifi_reset_test)
{
	/**
	 * <b>scenario</b>: Cannot send reset command.<br>
	 * <b>expected</b>: RETURN_NOK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(wifi_reset(), RETURN_NOK);

	/**
	 * <b>scenario</b>: Correct sequence.<br>
	 * <b>expected</b>: RETURN_OK returned.<br>
	 * ************************************************
	 */
	EXPECT_CALL(*uartengineMock, uartengine_send_string(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*ts_mock, ts_wait(_));
	EXPECT_CALL(*uartengineMock, uartengine_clear_rx());
	EXPECT_EQ(wifi_reset(), RETURN_OK);

}
