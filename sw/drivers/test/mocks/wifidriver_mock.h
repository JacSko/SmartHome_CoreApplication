#ifndef _BTENGINE_MOCK_H_
#define _BTENGINE_MOCK_H_

#include "wifi_driver.h"
#include "gmock/gmock.h"

struct wifiDriverMock
{
	MOCK_METHOD1(wifi_initialize, RET_CODE(const WIFI_UART_Config*));
	MOCK_METHOD2(wifi_connect_to_network, RET_CODE(const char*, const char*));
	MOCK_METHOD0(wifi_reset, RET_CODE());
	MOCK_METHOD0(wifi_disconnect_from_network, RET_CODE());
	MOCK_METHOD1(wifi_set_mac_address, RET_CODE(const char*));
	MOCK_METHOD1(wifi_open_server, RET_CODE(uint16_t));
	MOCK_METHOD0(wifi_close_server, RET_CODE());
	MOCK_METHOD1(wifi_allow_multiple_clients, RET_CODE(uint8_t state));
	MOCK_METHOD3(wifi_send_data, RET_CODE(ServerClientID, const char*, uint16_t));
	MOCK_METHOD1(wifi_set_ip_address, RET_CODE(IPAddress*));
	MOCK_METHOD1(wifi_get_ip_address, RET_CODE(IPAddress*));
	MOCK_METHOD2(wifi_get_time, RET_CODE(const char*, TimeItem*));
	MOCK_METHOD2(wifi_get_current_network_name, RET_CODE(char*, uint8_t size));
	MOCK_METHOD1(wifi_request_client_details, RET_CODE(ClientID*));
	MOCK_METHOD1(wifi_register_client_event_callback, RET_CODE(void(*)(ClientEvent, ServerClientID, const char*)));
	MOCK_METHOD0(wifi_unregister_client_event_callback, void());
	MOCK_METHOD0(wifi_deinitialize, void());
};

wifiDriverMock* wifi_driver_mock;

void mock_wifidriver_init()
{
	wifi_driver_mock = new wifiDriverMock;
}

void mock_wifidriver_deinit()
{
	delete wifi_driver_mock;
}

RET_CODE wifi_initialize(const WIFI_UART_Config* config)
{
	return wifi_driver_mock->wifi_initialize(config);
}
RET_CODE wifi_connect_to_network(const char* ssid, const char* password)
{
	return wifi_driver_mock->wifi_connect_to_network(ssid, password);
}
RET_CODE wifi_reset()
{
	return wifi_driver_mock->wifi_reset();
}
RET_CODE wifi_disconnect_from_network()
{
	return wifi_driver_mock->wifi_disconnect_from_network();
}
RET_CODE wifi_set_mac_address(const char* mac)
{
	return wifi_driver_mock->wifi_set_mac_address(mac);
}
RET_CODE wifi_open_server(uint16_t port)
{
	return wifi_driver_mock->wifi_open_server(port);
}
RET_CODE wifi_close_server()
{
	return wifi_driver_mock->wifi_close_server();
}
RET_CODE wifi_allow_multiple_clients(uint8_t state)
{
	return wifi_driver_mock->wifi_allow_multiple_clients(state);
}
RET_CODE wifi_send_data(ServerClientID id, const char* data, uint16_t size)
{
	return wifi_driver_mock->wifi_send_data(id, data, size);
}
RET_CODE wifi_set_ip_address(IPAddress* ip_address)
{
	return wifi_driver_mock->wifi_set_ip_address(ip_address);
}
RET_CODE wifi_get_ip_address(IPAddress* ip_address)
{
	return wifi_driver_mock->wifi_get_ip_address(ip_address);
}
RET_CODE wifi_get_time(const char* ntp_server, TimeItem* item)
{
	return wifi_driver_mock->wifi_get_time(ntp_server, item);
}
RET_CODE wifi_get_current_network_name(char* buffer, uint8_t size)
{
	return wifi_driver_mock->wifi_get_current_network_name(buffer, size);
}
RET_CODE wifi_request_client_details(ClientID* client)
{
	return wifi_driver_mock->wifi_request_client_details(client);
}
RET_CODE wifi_register_client_event_callback(void(*callback)(ClientEvent ev, ServerClientID id, const char* data))
{
	return wifi_driver_mock->wifi_register_client_event_callback(callback);
}
void wifi_unregister_client_event_callback()
{
	wifi_driver_mock->wifi_unregister_client_event_callback();
}
void wifi_deinitialize()
{
	wifi_driver_mock->wifi_deinitialize();
}


#endif
