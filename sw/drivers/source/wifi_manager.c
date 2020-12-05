#include "wifi_manager.h"
#include "Logger.h"


void wifimgr_on_client_event(ClientEvent ev, ServerClientID id, const char* data);

uint8_t WIFIMGR_MAX_CLIENTS = 2;


char wifi_cur_ssid [32] = "NIE_KRADNIJ_INTERNETU!!!";
char wifi_cur_pass [64] = "radionet0098";
char wifi_ntp_server [100] = "194.146.251.100";
uint16_t wifi_server_port = 4444;

typedef struct WifiMgr
{
	uint8_t clients_cnt;
	ClientID* clients;
	uint8_t wifi_connected;
	uint8_t server_running;
	uint16_t server_port;
	char* ssid;
	char* pass;
} WifiMgr;


WifiMgr wifi_mgr;

RET_CODE wifimgr_initialize()
{
	wifi_mgr.ssid = wifi_cur_ssid;
	wifi_mgr.pass = wifi_cur_pass;

	wifi_mgr.clients_cnt = 0;
	wifi_mgr.wifi_connected = 0;
	wifi_mgr.server_port = wifi_server_port;
	wifi_mgr.server_running = 0;

	RET_CODE result = RETURN_NOK;

	do
	{
		if (wifi_initialize() != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "Cannot init wifi driver");
			break;
		}
		if (wifi_connect_to_network(wifi_mgr.ssid, wifi_mgr.pass) != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "Cannot connect to network!");
			break;
		}

		wifi_mgr.wifi_connected = 1;

		if (wifi_allow_multiple_clients(0) != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "Cannot disable multiclient mode!");
			break;
		}
		TimeItem tim;

		if (wifi_get_time("194.146.251.100", &tim) != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "Cannot get NTP time!");
			break;
		}

		if (time_set_utc(&tim) != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "Cannot set system time!");
			break;
		}

		if (wifi_allow_multiple_clients(1) != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "Cannot enable multiclient mode");
			break;
		}

		if (wifi_open_server(wifi_mgr.server_port) != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "Cannot start UDP server");
			break;
		}
		wifi_mgr.server_running = 1;
		if (wifi_register_client_event_callback(&wifimgr_on_client_event) != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "Cannot subscribe for client events");
			break;
		}
		result = RETURN_OK;

	}
	while(0);

	return result;
}

void wifimgr_on_client_event(ClientEvent ev, ServerClientID id, const char* data)
{

}

RET_CODE wifimgr_set_network_data(const char* ssid, const char* pass)
{

}

RET_CODE wifimgr_set_mac_address(const char* mac)
{

}

RET_CODE wifimgr_set_ip_address(const char* address)
{

}

RET_CODE wifimgr_set_server_port(uint16_t port)
{

}

RET_CODE wifimgr_send_data(ServerClientID id, const char* data)
{

}

RET_CODE wifimgr_broadcast_data(const char* data)
{

}

RET_CODE wifimgr_get_time(TimeItem* item)
{

}

uint8_t wifimgr_count_clients()
{

}

RET_CODE wifimgr_register_data_callbacl(void(*callback)(ServerClientID id, const char* data))
{

}

void wifimgr_unregister_data_callback(void(*callback)(ServerClientID id, const char* data))
{

}

RET_CODE wifimgr_set_max_clients(uint8_t max)
{

}

void wifimgr_deinitialize()
{

}
