#ifndef _WIFI_MANAGER_MOCK_H_
#define _WIFI_MANAGER_MOCK_H_

#include "wifi_driver.h"
#include "gmock/gmock.h"

struct wifiMgrMock
{
	MOCK_METHOD1(wifimgr_initialize, RET_CODE(const WIFI_UART_Config*));
	MOCK_METHOD2(wifimgr_set_network_data, RET_CODE(const char*, const char*));
	MOCK_METHOD1(wifimgr_set_ntp_server, RET_CODE(const char*));
	MOCK_METHOD1(wifimgr_set_ip_address, RET_CODE(const char*));
	MOCK_METHOD1(wifimgr_set_server_port, RET_CODE(uint16_t));
	MOCK_METHOD2(wifimgr_send_data, RET_CODE(ServerClientID, const char*));
   MOCK_METHOD3(wifimgr_send_bytes, RET_CODE(ServerClientID, const uint8_t*, uint16_t size));
	MOCK_METHOD1(wifimgr_broadcast_data, RET_CODE(const char*));
	MOCK_METHOD2(wifimgr_broadcast_bytes, RET_CODE(const uint8_t*, uint16_t));
	MOCK_METHOD1(wifimgr_get_time, RET_CODE(TimeItem*));
	MOCK_METHOD0(wifimgr_count_clients, uint8_t());
	MOCK_METHOD1(wifimgr_get_ip_address, RET_CODE(IPAddress*));
	MOCK_METHOD2(wifimgr_get_network_name, RET_CODE(char*, uint8_t));
	MOCK_METHOD1(wifimgr_get_clients_details, uint8_t(ClientID*));
	MOCK_METHOD0(wifi_get_max_clients, uint8_t());
	MOCK_METHOD0(wifimgr_get_ntp_server, const char*());
	MOCK_METHOD0(wifimgr_get_server_port, uint16_t());
	MOCK_METHOD1(wifimgr_register_data_callback, RET_CODE(void(*callback)(ServerClientID, const char*)));
	MOCK_METHOD1(wifimgr_unregister_data_callback, RET_CODE(void(*callback)(ServerClientID, const char*)));
	MOCK_METHOD0(wifimgr_reset, RET_CODE());
	MOCK_METHOD0(wifimgr_deinitialize, void());

};

wifiMgrMock* wifimgr_mock;

void mock_wifimgr_init()
{
	wifimgr_mock = new wifiMgrMock;
}

void mock_wifimgr_deinit()
{
	delete wifimgr_mock;
}

RET_CODE wifimgr_initialize(const WIFI_UART_Config* config)
{
	return wifimgr_mock->wifimgr_initialize(config);
}
RET_CODE wifimgr_set_network_data(const char* ssid, const char* pass)
{
	return wifimgr_mock->wifimgr_set_network_data(ssid, pass);
}
RET_CODE wifimgr_set_ntp_server(const char* server)
{
	return wifimgr_mock->wifimgr_set_ntp_server(server);
}
RET_CODE wifimgr_set_ip_address(const char* address)
{
	return wifimgr_mock->wifimgr_set_ip_address(address);
}
RET_CODE wifimgr_set_server_port(uint16_t port)
{
	return wifimgr_mock->wifimgr_set_server_port(port);
}
RET_CODE wifimgr_send_data(ServerClientID id, const char* data)
{
	return wifimgr_mock->wifimgr_send_data(id, data);
}
RET_CODE wifimgr_send_bytes(ServerClientID id, const uint8_t* data, uint16_t size)
{
   return wifimgr_mock->wifimgr_send_bytes(id, data, size);
}
RET_CODE wifimgr_broadcast_data(const char* data)
{
	return wifimgr_mock->wifimgr_broadcast_data(data);
}
RET_CODE wifimgr_broadcast_bytes(const uint8_t* data, uint16_t size)
{
   return wifimgr_mock->wifimgr_broadcast_bytes(data, size);
}
RET_CODE wifimgr_get_time(TimeItem* item)
{
	return wifimgr_mock->wifimgr_get_time(item);
}
uint8_t wifimgr_count_clients()
{
	return wifimgr_mock->wifimgr_count_clients();
}
RET_CODE wifimgr_get_ip_address(IPAddress* item)
{
	return wifimgr_mock->wifimgr_get_ip_address(item);
}
RET_CODE wifimgr_get_network_name(char* buf, uint8_t buf_size)
{
	return wifimgr_mock->wifimgr_get_network_name(buf, buf_size);
}
uint8_t wifimgr_get_clients_details(ClientID* buffer)
{
	return wifimgr_mock->wifimgr_get_clients_details(buffer);
}
uint8_t wifi_get_max_clients()
{
	return wifimgr_mock->wifi_get_max_clients();
}
RET_CODE wifimgr_register_data_callback(void(*callback)(ServerClientID id, const char* data))
{
	return wifimgr_mock->wifimgr_register_data_callback(callback);
}
void wifimgr_unregister_data_callback(void(*callback)(ServerClientID id, const char* data))
{
	wifimgr_mock->wifimgr_unregister_data_callback(callback);
}
RET_CODE wifimgr_reset()
{
	return wifimgr_mock->wifimgr_reset();
}
const char* wifimgr_get_ntp_server()
{
	return wifimgr_mock->wifimgr_get_ntp_server();
}
uint16_t wifimgr_get_server_port()
{
	return wifimgr_mock->wifimgr_get_server_port();
}
void wifimgr_deinitialize()
{
	wifimgr_mock->wifimgr_deinitialize();
}



#endif
