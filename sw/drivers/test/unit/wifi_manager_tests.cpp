#include "gtest/gtest.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/wifi_manager.c"
#ifdef __cplusplus
}
#endif

#include "wifidriver_mock.h"
#include "time_counter_mock.h"
#include "logger_mock.h"
/**
 * @brief Unit test of wifi manager.
 *
 * All tests that verify behavior of WiFi manager module
 *
 * @file wifi_manager_tests.cpp
 * @author  Jacek Skowronek
 * @date    06/12/2020
 */


using namespace ::testing;

struct callbackMock
{
	MOCK_METHOD2(callback, void(ServerClientID, const char*));
};

callbackMock* callMock;

void fake_callback(ServerClientID id, const char* data)
{
	callMock->callback(id, data);
}


struct wifiMgrFixture : public ::testing::Test
{

	virtual void SetUp()
	{
		mock_logger_init();
		mock_wifidriver_init();
		mock_time_counter_init();
		callMock = new callbackMock();
	}

	virtual void TearDown()
	{
		mock_logger_deinit();
		mock_wifidriver_deinit();
		mock_time_counter_deinit();
		delete callMock;
	}

	virtual void setup_test_subject()
	{
		EXPECT_CALL(*wifi_driver_mock, wifi_initialize(_)).WillOnce(Return(RETURN_OK));
		EXPECT_CALL(*wifi_driver_mock, wifi_set_ip_address(_)).WillOnce(Return(RETURN_OK));
		EXPECT_CALL(*wifi_driver_mock, wifi_connect_to_network(_,_)).WillOnce(Return(RETURN_OK));
		EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(0)).WillOnce(Return(RETURN_OK));
		EXPECT_CALL(*wifi_driver_mock, wifi_get_time(_,_)).WillOnce(Return(RETURN_OK));
		EXPECT_CALL(*time_cnt_mock, time_set_utc(_)).WillOnce(Return(RETURN_OK));
		EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(1)).WillOnce(Return(RETURN_OK));
		EXPECT_CALL(*wifi_driver_mock, wifi_open_server(_)).WillOnce(Return(RETURN_OK));
		EXPECT_CALL(*wifi_driver_mock, wifi_register_client_event_callback(_)).WillOnce(Return(RETURN_OK));
		EXPECT_EQ(RETURN_OK, wifimgr_initialize(&uart_config));
		Mock::VerifyAndClearExpectations(wifi_driver_mock);
		Mock::VerifyAndClearExpectations(time_cnt_mock);
	}
	virtual void teardown_test_subject()
	{
		EXPECT_CALL(*wifi_driver_mock, wifi_reset());
		EXPECT_CALL(*wifi_driver_mock, wifi_deinitialize());
		wifimgr_deinitialize();
		Mock::VerifyAndClearExpectations(wifi_driver_mock);
		Mock::VerifyAndClearExpectations(time_cnt_mock);
	}
	const WIFI_UART_Config uart_config = {115200, 1024, 512};
};

/**
 * @test WiFi manager initialization
 */
TEST_F(wifiMgrFixture, manager_initialization)
{
	/**
	 * @<b>scenario<\b>: Module initialization - cannot initialize wifi driver.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_CALL(*wifi_driver_mock, wifi_initialize(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifimgr_initialize(&uart_config));

	/**
	 * @<b>scenario<\b>: Module initialization - cannot set station IP address.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_CALL(*wifi_driver_mock, wifi_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_set_ip_address(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifimgr_initialize(&uart_config));

	/**
	 * @<b>scenario<\b>: Module initialization - cannot connect to network.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_CALL(*wifi_driver_mock, wifi_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_set_ip_address(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_connect_to_network(_,_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifimgr_initialize(&uart_config));

	/**
	 * @<b>scenario<\b>: Module initialization - cannot disable multiclient mode.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_CALL(*wifi_driver_mock, wifi_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_set_ip_address(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_connect_to_network(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(0)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifimgr_initialize(&uart_config));

	/**
	 * @<b>scenario<\b>: Module initialization - cannot get NTP time, cannot set multiclient mode
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_CALL(*wifi_driver_mock, wifi_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_set_ip_address(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_connect_to_network(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(0)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_get_time(_,_)).WillOnce(Return(RETURN_NOK))
													  .WillOnce(Return(RETURN_NOK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(1)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifimgr_initialize(&uart_config));

	/**
	 * @<b>scenario<\b>: Module initialization - cannot open server, NTP time got correctly but cannot set
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_CALL(*wifi_driver_mock, wifi_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_set_ip_address(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_connect_to_network(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(0)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_get_time(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_set_utc(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(1)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_open_server(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifimgr_initialize(&uart_config));

	/**
	 * @<b>scenario<\b>: Module initialization - cannot subscribe for client notifications
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_CALL(*wifi_driver_mock, wifi_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_set_ip_address(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_connect_to_network(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(0)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_get_time(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_set_utc(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(1)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_open_server(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_register_client_event_callback(_)).WillOnce(Return(RETURN_NOK));
	EXPECT_EQ(RETURN_NOK, wifimgr_initialize(&uart_config));

	/**
	 * @<b>scenario<\b>: Module initialization - performed successfully
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_CALL(*wifi_driver_mock, wifi_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_set_ip_address(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_connect_to_network(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(0)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_get_time(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_set_utc(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(1)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_open_server(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_register_client_event_callback(_)).WillOnce(Return(RETURN_OK));
	EXPECT_EQ(RETURN_OK, wifimgr_initialize(&uart_config));


	EXPECT_CALL(*wifi_driver_mock, wifi_reset());
	EXPECT_CALL(*wifi_driver_mock, wifi_deinitialize());
	wifimgr_deinitialize();

}

/**
 * @test WiFi manager behavior on client connect/disconnect/data receive
 */
TEST_F(wifiMgrFixture, manager_client_handling)
{
	setup_test_subject();
	uint8_t clientid = 0;
	const char string [] = "TEST_STRING";

	/**
	 * @<b>scenario<\b>: First client connected to server.
	 * @<b>expected<\b>: Clients counter incremented, client details received.
	 */

	EXPECT_CALL(*wifi_driver_mock, wifi_request_client_details(_)).WillOnce(Invoke([&](ClientID* client)->RET_CODE
			{
				client->address.ip_address[0] = 1;
				client->address.ip_address[1] = 2;
				client->address.ip_address[2] = 3;
				client->address.ip_address[3] = 4;
				EXPECT_EQ(client->id, clientid);
				return RETURN_OK;
			}));

	wifimgr_on_client_event(CLIENT_CONNECTED, clientid, NULL);
	EXPECT_EQ(wifimgr_count_clients(), 1);

	/**
	 * @<b>scenario<\b>: Second client connected.
	 * @<b>expected<\b>: Clients counter incremented, client details received.
	 */

	clientid = 1;
	EXPECT_CALL(*wifi_driver_mock, wifi_request_client_details(_)).WillOnce(Invoke([&](ClientID* client)->RET_CODE
			{
				client->address.ip_address[0] = 5;
				client->address.ip_address[1] = 6;
				client->address.ip_address[2] = 7;
				client->address.ip_address[3] = 8;
				EXPECT_EQ(client->id, clientid);
				return RETURN_OK;
			}));

	wifimgr_on_client_event(CLIENT_CONNECTED, clientid, NULL);
	EXPECT_EQ(wifimgr_count_clients(), 2);

	/**
	 * @<b>scenario<\b>: Third client connected.
	 * @<b>expected<\b>: Client rejected, due to max client limit = 2.
	 */

	clientid = 2;

	wifimgr_on_client_event(CLIENT_CONNECTED, clientid, NULL);
	EXPECT_EQ(wifimgr_count_clients(), 2);

	/**
	 * @<b>scenario<\b>: Getting connected clients details.
	 * @<b>expected<\b>: Clients data are correct.
	 */
	{
		const uint8_t max_client_count = 2;
		ClientID clients [max_client_count];
		EXPECT_EQ(max_client_count, wifimgr_get_clients_details(clients));
		EXPECT_EQ(clients[0].id, 0);
		EXPECT_EQ(clients[0].address.ip_address[0], 1);
		EXPECT_EQ(clients[0].address.ip_address[1], 2);
		EXPECT_EQ(clients[0].address.ip_address[2], 3);
		EXPECT_EQ(clients[0].address.ip_address[3], 4);
		EXPECT_EQ(clients[1].id, 1);
		EXPECT_EQ(clients[1].address.ip_address[0], 5);
		EXPECT_EQ(clients[1].address.ip_address[1], 6);
		EXPECT_EQ(clients[1].address.ip_address[2], 7);
		EXPECT_EQ(clients[1].address.ip_address[3], 8);
	}

	/**
	 * @<b>scenario<\b>: Sending data to client 1.
	 * @<b>expected<\b>: Data send successfully.
	 */
	clientid = 0;
	EXPECT_CALL(*wifi_driver_mock, wifi_send_data(clientid,_,strlen(string))).WillOnce(Return(RETURN_OK));
	EXPECT_EQ(RETURN_OK, wifimgr_send_data(clientid, string));

	/**
	 * @<b>scenario<\b>: Sending data to client 2.
	 * @<b>expected<\b>: Data send successfully.
	 */
	clientid = 1;
	EXPECT_CALL(*wifi_driver_mock, wifi_send_data(clientid,_,strlen(string))).WillOnce(Return(RETURN_OK));
	EXPECT_EQ(RETURN_OK, wifimgr_send_data(clientid, string));

	/**
	 * @<b>scenario<\b>: Broadcast data to all clients.
	 * @<b>expected<\b>: Data send successfully.
	 */
	EXPECT_CALL(*wifi_driver_mock, wifi_send_data(0,_,strlen(string))).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_send_data(1,_,strlen(string))).WillOnce(Return(RETURN_OK));
	EXPECT_EQ(RETURN_OK, wifimgr_broadcast_data(string));

	/**
	 * @<b>scenario<\b>: Data from client 1 received, no callback registered.
	 * @<b>expected<\b>: Callback not called.
	 */
	EXPECT_CALL(*callMock, callback(_,_)).Times(0);
	wifimgr_on_client_event(CLIENT_DATA, clientid, string);

	/**
	 * @<b>scenario<\b>: Registering callback, data from client 2 received.
	 * @<b>expected<\b>: Callback called.
	 */
	clientid = 1;
	EXPECT_EQ(RETURN_OK, wifimgr_register_data_callback(&fake_callback));
	EXPECT_CALL(*callMock, callback(clientid,_)).Times(1);
	wifimgr_on_client_event(CLIENT_DATA, clientid, "DATA FROM 2");

	/**
	 * @<b>scenario<\b>: Client 1 disconnected.
	 * @<b>expected<\b>: Client counter changed.
	 */
	clientid = 0;
	wifimgr_on_client_event(CLIENT_DISCONNECTED, clientid, NULL);
	EXPECT_EQ(wifimgr_count_clients(), 1);

	/**
	 * @<b>scenario<\b>: Getting connected clients details after disconnection of another client.
	 * @<b>expected<\b>: Clients data are correct.
	 */
	{
		const uint8_t max_client_count = 2;
		ClientID clients [max_client_count];
		EXPECT_EQ(1, wifimgr_get_clients_details(clients));
		EXPECT_EQ(clients[0].id, 1);
		EXPECT_EQ(clients[0].address.ip_address[0], 5);
		EXPECT_EQ(clients[0].address.ip_address[1], 6);
		EXPECT_EQ(clients[0].address.ip_address[2], 7);
		EXPECT_EQ(clients[0].address.ip_address[3], 8);

	}

	/**
	 * @<b>scenario<\b>: Sending data to client 1 - which is disconnected.
	 * @<b>expected<\b>: Data not sent.
	 */
	clientid = 0;
	EXPECT_CALL(*wifi_driver_mock, wifi_send_data(_,_,_)).Times(0);
	EXPECT_EQ(RETURN_NOK, wifimgr_send_data(clientid, string));

	/**
	 * @<b>scenario<\b>: Broadcast data to all clients - currently only Client2 is connected.
	 * @<b>expected<\b>: Data send successfully.
	 */
	EXPECT_CALL(*wifi_driver_mock, wifi_send_data(0,_,_)).Times(0);
	EXPECT_CALL(*wifi_driver_mock, wifi_send_data(1,_,strlen(string))).WillOnce(Return(RETURN_OK));
	EXPECT_EQ(RETURN_OK, wifimgr_broadcast_data(string));

	/**
	 * @<b>scenario<\b>: Client 2 disconnected.
	 * @<b>expected<\b>: Client counter changed.
	 */
	clientid = 1;
	wifimgr_on_client_event(CLIENT_DISCONNECTED, clientid, NULL);
	EXPECT_EQ(wifimgr_count_clients(), 0);

	/**
	 * @<b>scenario<\b>: Broadcast data to all clients - currently no clients connected.
	 * @<b>expected<\b>: Data send successfully.
	 */
	EXPECT_CALL(*wifi_driver_mock, wifi_send_data(0,_,_)).Times(0);
	EXPECT_CALL(*wifi_driver_mock, wifi_send_data(1,_,_)).Times(0);
	EXPECT_EQ(RETURN_NOK, wifimgr_broadcast_data(string));

	teardown_test_subject();

}

/**
 * @test WiFi manager - changing current wifi network data
 */
TEST_F(wifiMgrFixture, changing_network_name)
{
	setup_test_subject();
	/**
	 * @<b>scenario<\b>: SSID is NULL, PASS is NULL.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_EQ(RETURN_NOK, wifimgr_set_network_data(NULL, ""));
	EXPECT_EQ(RETURN_NOK, wifimgr_set_network_data("", NULL));

	/**
	 * @<b>scenario<\b>: SSID is too long, PASS is too long.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_EQ(RETURN_NOK, wifimgr_set_network_data("THIS SHOULD HAVE MORE THAN 32 CHARS",""));
	EXPECT_EQ(RETURN_NOK, wifimgr_set_network_data("", "IT IS HARD TO MAKE THIS STRING LONGER THAN 64 BYTES BUT WE ARE TRYING"));

	/**
	 * @<b>scenario<\b>: SSID change.
	 * @<b>expected<\b>: SSID should be changed, but it shall be done after reset.
	 */
	EXPECT_EQ(RETURN_OK, wifimgr_set_network_data("NEW_SSID","NEW_PASS"));

	EXPECT_CALL(*wifi_driver_mock, wifi_reset());
	EXPECT_CALL(*wifi_driver_mock, wifi_deinitialize());

	EXPECT_CALL(*wifi_driver_mock, wifi_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_set_ip_address(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_connect_to_network(_,_)).WillOnce(Invoke([&](const char* ssid, const char* pass) -> RET_CODE
			{
				EXPECT_STREQ("NEW_SSID",ssid);
				EXPECT_STREQ("NEW_PASS",pass);
				return RETURN_OK;
			}));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(0)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_get_time(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_set_utc(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(1)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_open_server(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_register_client_event_callback(_)).WillOnce(Return(RETURN_OK));

	EXPECT_EQ(RETURN_OK, wifimgr_reset());

	teardown_test_subject();
}

/**
 * @test WiFi manager - changing current ip address
 */
TEST_F(wifiMgrFixture, changing_ip_address)
{
	char address_nok []= "192.168.100";
	char address_ok []= "192.168.100.30";
	setup_test_subject();
	/**
	 * @<b>scenario<\b>: Address is NULL
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_EQ(RETURN_NOK, wifimgr_set_ip_address(NULL));;

	/**
	 * @<b>scenario<\b>: IP address has invalid format.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_EQ(RETURN_NOK, wifimgr_set_ip_address(address_nok));

	/**
	 * @<b>scenario<\b>: IP address has correct format.
	 * @<b>expected<\b>: IP address changed, but it shall be done after reset.
	 */
	EXPECT_EQ(RETURN_OK, wifimgr_set_ip_address(address_ok));

	EXPECT_CALL(*wifi_driver_mock, wifi_reset());
	EXPECT_CALL(*wifi_driver_mock, wifi_deinitialize());

	EXPECT_CALL(*wifi_driver_mock, wifi_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_set_ip_address(_)).WillOnce(Invoke([&](IPAddress* ip_address) -> RET_CODE
			{
				EXPECT_EQ(ip_address->ip_address[0], 192);
				EXPECT_EQ(ip_address->ip_address[1], 168);
				EXPECT_EQ(ip_address->ip_address[2], 100);
				EXPECT_EQ(ip_address->ip_address[3], 30);
				return RETURN_OK;
			}));
	EXPECT_CALL(*wifi_driver_mock, wifi_connect_to_network(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(0)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_get_time(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_set_utc(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(1)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_open_server(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_register_client_event_callback(_)).WillOnce(Return(RETURN_OK));

	EXPECT_EQ(RETURN_OK, wifimgr_reset());

	teardown_test_subject();
}

/**
 * @test WiFi manager - changing server port
 */
TEST_F(wifiMgrFixture, changing_server_port)
{
	uint16_t port_too_low = 999;
	uint16_t port_too_high = 10000;
	uint16_t port_ok = 5555;

	setup_test_subject();
	/**
	 * @<b>scenario<\b>: Port is invalid
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_EQ(RETURN_NOK, wifimgr_set_server_port(port_too_low));;
	EXPECT_EQ(RETURN_NOK, wifimgr_set_server_port(port_too_high));;

	/**
	 * @<b>scenario<\b>: Port is correct and is changed.
	 * @<b>expected<\b>: Port changed, but it shall be done after reset.
	 */
	EXPECT_EQ(RETURN_OK, wifimgr_set_server_port(port_ok));

	EXPECT_CALL(*wifi_driver_mock, wifi_reset());
	EXPECT_CALL(*wifi_driver_mock, wifi_deinitialize());

	EXPECT_CALL(*wifi_driver_mock, wifi_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_set_ip_address(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_connect_to_network(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(0)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_get_time(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_set_utc(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(1)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_open_server(port_ok)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_register_client_event_callback(_)).WillOnce(Return(RETURN_OK));

	EXPECT_EQ(RETURN_OK, wifimgr_reset());

	teardown_test_subject();
}

/**
 * @test WiFi manager - changing NTP server
 */
TEST_F(wifiMgrFixture, changing_ntp_server)
{
	char new_ntp_server [] = "www.new_server.pl";
	setup_test_subject();
	/**
	 * @<b>scenario<\b>: Server is NULL.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_EQ(RETURN_NOK, wifimgr_set_ntp_server(NULL));
	/**
	 * @<b>scenario<\b>: Port is correct and is changed.
	 * @<b>expected<\b>: Port changed, but it shall be done after reset.
	 */
	EXPECT_EQ(RETURN_OK, wifimgr_set_ntp_server(new_ntp_server));

	EXPECT_CALL(*wifi_driver_mock, wifi_reset());
	EXPECT_CALL(*wifi_driver_mock, wifi_deinitialize());

	EXPECT_CALL(*wifi_driver_mock, wifi_initialize(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_set_ip_address(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_connect_to_network(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(0)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_get_time(_,_)).WillOnce(Invoke([&](const char* server, TimeItem* item) -> RET_CODE
			{
				EXPECT_STREQ(server, new_ntp_server);
				return RETURN_OK;
			}));
	EXPECT_CALL(*time_cnt_mock, time_set_utc(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(1)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_open_server(_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_register_client_event_callback(_)).WillOnce(Return(RETURN_OK));

	EXPECT_EQ(RETURN_OK, wifimgr_reset());

	teardown_test_subject();
}

/**
 * @test WiFi manager - getting time incorrect sequences
 */
TEST_F(wifiMgrFixture, reading_time_from_ntp_server_negative_cases)
{
	TimeItem item = {};

	EXPECT_CALL(*wifi_driver_mock, wifi_initialize(_)).WillOnce(Return(RETURN_OK))
													 .WillOnce(Return(RETURN_OK))
													 .WillOnce(Return(RETURN_OK))
													 .WillOnce(Return(RETURN_OK))
													 .WillOnce(Return(RETURN_OK))
													 .WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_set_ip_address(_)).WillOnce(Return(RETURN_OK))
														  .WillOnce(Return(RETURN_OK))
														  .WillOnce(Return(RETURN_OK))
														  .WillOnce(Return(RETURN_OK))
														  .WillOnce(Return(RETURN_OK))
														  .WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_connect_to_network(_,_)).WillOnce(Return(RETURN_OK))
																.WillOnce(Return(RETURN_OK))
																.WillOnce(Return(RETURN_OK))
																.WillOnce(Return(RETURN_OK))
																.WillOnce(Return(RETURN_OK))
																.WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(0)).WillOnce(Return(RETURN_OK))
																  .WillOnce(Return(RETURN_OK))
																  .WillOnce(Return(RETURN_NOK))
																  .WillOnce(Return(RETURN_OK))
																  .WillOnce(Return(RETURN_OK))
																  .WillOnce(Return(RETURN_OK))
																  .WillOnce(Return(RETURN_OK))
																  .WillOnce(Return(RETURN_OK))
																  .WillOnce(Return(RETURN_OK))
																  .WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_get_time(_,_)).WillOnce(Return(RETURN_OK))
													  .WillOnce(Return(RETURN_OK))
													  .WillOnce(Return(RETURN_OK))
													  .WillOnce(Return(RETURN_NOK))
													  .WillOnce(Return(RETURN_OK))
													  .WillOnce(Return(RETURN_OK))
													  .WillOnce(Return(RETURN_OK))
													  .WillOnce(Return(RETURN_OK))
													  .WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*time_cnt_mock, time_set_utc(_)).WillOnce(Return(RETURN_OK))
												.WillOnce(Return(RETURN_OK))
												.WillOnce(Return(RETURN_OK))
												.WillOnce(Return(RETURN_OK))
												.WillOnce(Return(RETURN_OK))
												.WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(1)).WillOnce(Return(RETURN_OK))
																  .WillOnce(Return(RETURN_OK))
																  .WillOnce(Return(RETURN_OK))
																  .WillOnce(Return(RETURN_OK))
																  .WillOnce(Return(RETURN_NOK))
																  .WillOnce(Return(RETURN_OK))
																  .WillOnce(Return(RETURN_OK))
																  .WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_open_server(_)).WillOnce(Return(RETURN_OK))
													   .WillOnce(Return(RETURN_OK))
													   .WillOnce(Return(RETURN_OK))
													   .WillOnce(Return(RETURN_OK))
													   .WillOnce(Return(RETURN_OK))
													   .WillOnce(Return(RETURN_NOK))
													   .WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_register_client_event_callback(_)).WillOnce(Return(RETURN_OK))
																		  .WillOnce(Return(RETURN_OK))
																		  .WillOnce(Return(RETURN_OK))
																		  .WillOnce(Return(RETURN_OK))
																		  .WillOnce(Return(RETURN_OK))
																		  .WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_reset()).WillRepeatedly(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_deinitialize()).Times(5);

	EXPECT_CALL(*wifi_driver_mock, wifi_close_server()).WillOnce(Return(RETURN_NOK))
													   .WillOnce(Return(RETURN_OK))
													   .WillOnce(Return(RETURN_OK))
													   .WillOnce(Return(RETURN_OK))
													   .WillOnce(Return(RETURN_OK));

	EXPECT_EQ(RETURN_OK, wifimgr_initialize(&uart_config));

	/**
	 * @<b>scenario<\b>: Cannot close server getting NTP time.
	 * @<b>expected<\b>: Server restarted.
	 */
	EXPECT_EQ(RETURN_NOK, wifimgr_get_time(&item));	/* cannot close server */
	/**
	 * @<b>scenario<\b>: Cannot disable multiclient mode getting NTP time.
	 * @<b>expected<\b>: Server restarted.
	 */
	EXPECT_EQ(RETURN_NOK, wifimgr_get_time(&item));	/* cannot disable multiclient */
	/**
	 * @<b>scenario<\b>: Cannot get NTP time.
	 * @<b>expected<\b>: Server restarted.
	 */
	EXPECT_EQ(RETURN_NOK, wifimgr_get_time(&item));	/* cannot get time */
	/**
	 * @<b>scenario<\b>: Cannot enable multiclient after getting NTP time.
	 * @<b>expected<\b>: Server restarted.
	 */
	EXPECT_EQ(RETURN_NOK, wifimgr_get_time(&item));	/* cannot enable multiclient */
	/**
	 * @<b>scenario<\b>: Cannot enable server after getting NTP time.
	 * @<b>expected<\b>: Server restarted.
	 */
	EXPECT_EQ(RETURN_NOK, wifimgr_get_time(&item));	/* cannot enable server */

}

/**
 * @test WiFi manager - getting time correct sequence
 */
TEST_F(wifiMgrFixture, reading_time_from_ntp_server_positive_cases)
{
	setup_test_subject();
	uint8_t clientid = 0;
	TimeItem item = {};

	/**
	 * @<b>scenario<\b>: Getting NTP time when client is connected.
	 * @<b>expected<\b>: Client disconnected, server opened again.
	 */

	EXPECT_CALL(*wifi_driver_mock, wifi_request_client_details(_)).WillOnce(Invoke([&](ClientID* client)->RET_CODE
			{
				client->address.ip_address[0] = 1;
				client->address.ip_address[1] = 2;
				client->address.ip_address[2] = 3;
				client->address.ip_address[3] = 4;
				EXPECT_EQ(client->id, clientid);
				return RETURN_OK;
			}));

	wifimgr_on_client_event(CLIENT_CONNECTED, clientid, NULL);
	EXPECT_EQ(wifimgr_count_clients(), 1);

	EXPECT_CALL(*wifi_driver_mock, wifi_close_server()).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(0)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_get_time(_,_)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_allow_multiple_clients(1)).WillOnce(Return(RETURN_OK));
	EXPECT_CALL(*wifi_driver_mock, wifi_open_server(_)).WillOnce(Return(RETURN_OK));

	EXPECT_EQ(RETURN_OK, wifimgr_get_time(&item));
	EXPECT_EQ(wifimgr_count_clients(), 0);
}

/**
 * @test WiFi manager - register, unregister callback
 */
TEST_F(wifiMgrFixture, callback_register_unregister)
{
	/**
	 * @<b>scenario<\b>: Registering callback when there is no callback registered.
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	wifimgr_unregister_data_callback(&fake_callback);
	EXPECT_EQ(RETURN_OK, wifimgr_register_data_callback(&fake_callback));

	/**
	 * @<b>scenario<\b>: Unregistering callback when there is callback registered.
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	EXPECT_EQ(RETURN_NOK, wifimgr_register_data_callback(&fake_callback));

	/**
	 * @<b>scenario<\b>: Unregister and register callback when there is callback registered.
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	wifimgr_unregister_data_callback(&fake_callback);
	EXPECT_EQ(RETURN_OK, wifimgr_register_data_callback(&fake_callback));
}

/**
 * @test WiFi manager - get IP address test
 */
TEST_F(wifiMgrFixture, get_ip_address_test)
{
	/**
	 * @<b>scenario<\b>: Getting current IP address.
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	IPAddress item = {};
	IPAddress result = {};
	item.ip_address[0] = 1;
	item.ip_address[1] = 2;
	item.ip_address[2] = 3;
	item.ip_address[3] = 4;

	EXPECT_CALL(*wifi_driver_mock, wifi_get_ip_address(_)).WillOnce(Invoke([&](IPAddress* addr) -> RET_CODE
			{
				*addr = item;
				return RETURN_OK;
			}));

	EXPECT_EQ(RETURN_OK, wifimgr_get_ip_address(&result));
	EXPECT_EQ(result.ip_address[0], 1);
	EXPECT_EQ(result.ip_address[1], 2);
	EXPECT_EQ(result.ip_address[2], 3);
	EXPECT_EQ(result.ip_address[3], 4);
}

/**
 * @test WiFi manager - get name of ssid test
 */
TEST_F(wifiMgrFixture, get_ssid_name)
{
	/**
	 * @<b>scenario<\b>: Getting current network name.
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	char result [32];
	char name [32] = "TEST_SSID_NAME";


	EXPECT_CALL(*wifi_driver_mock, wifi_get_current_network_name(_,_)).WillOnce(Invoke([&](char* ssid, uint8_t size) -> RET_CODE
			{
				for (uint8_t i = 0; i < size; i++)
				{
					ssid[i] = name[i];
				}
				return RETURN_OK;
			}));

	EXPECT_EQ(RETURN_OK, wifimgr_get_network_name(result, 32));
	EXPECT_STREQ(result, name);
}

