#include "wifi_manager.h"
#include "Logger.h"
#include <stdlib.h>
#include <string.h>

void wifimgr_on_client_event(ClientEvent ev, ServerClientID id, const char* data);
void(*wifimgr_data_callback)(ServerClientID id, const char* data);

uint8_t WIFIMGR_MAX_CLIENTS = 2;


char wifi_cur_ssid [32] = "NIE_KRADNIJ_INTERNETU!!!";
char wifi_cur_pass [64] = "radionet0098";
IPAddress wifi_cur_ip_address;

char wifi_ntp_server [100] = "194.146.251.100";
uint16_t wifi_server_port = 4444;

typedef struct WiFiClient
{
	uint8_t connected;
	ClientID id;
} WiFiClient;

typedef struct WifiMgr
{
	uint8_t clients_cnt;
	WiFiClient* clients;
	uint8_t wifi_connected;
	uint8_t server_running;
	uint16_t server_port;
	char* ssid;
	char* pass;
} WifiMgr;


WifiMgr wifi_mgr;

RET_CODE wifimgr_initialize()
{
	logger_send(LOG_WIFI_MANAGER, __func__, "initializing wifimgr");
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

		if (wifi_set_ip_address(&wifi_cur_ip_address) != RETURN_OK)
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
		}
		else
		{
			if (time_set_utc(&tim) != RETURN_OK)
			{
				logger_send(LOG_ERROR, __func__, "Cannot set system time!");
			}
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

		wifi_mgr.clients = (WiFiClient*) malloc (sizeof(WiFiClient) * WIFIMGR_MAX_CLIENTS);
		if (wifi_mgr.clients)
		{
			result = RETURN_OK;
		}

	}
	while(0);

	logger_send_if(result == RETURN_OK, LOG_WIFI_MANAGER, __func__, "initialization OK");
	return result;
}

void wifimgr_on_client_event(ClientEvent ev, ServerClientID id, const char* data)
{
	switch (ev)
	{
	case CLIENT_CONNECTED:
		if (wifi_mgr.clients_cnt < WIFIMGR_MAX_CLIENTS)
		{
			wifi_mgr.clients_cnt++;
			ClientID client_data = {};
			if (wifi_request_client_details(&client_data) == RETURN_OK)
			{
				logger_send(LOG_WIFI_MANAGER, __func__, "client: id %d, ip: %d.%d.%d.%d", client_data.id, client_data.address.ip_address[0], client_data.address.ip_address[1],
																								 client_data.address.ip_address[2], client_data.address.ip_address[3]);
				wifi_mgr.clients[id].connected = 1;
				wifi_mgr.clients[id].id = client_data;

			}
			else
			{
				logger_send(LOG_ERROR, __func__, "Cannot get client details");
			}
		}
		break;
	case CLIENT_DISCONNECTED:
		wifi_mgr.clients_cnt--;
		wifi_mgr.clients[id].connected = 0;
		break;
	case CLIENT_DATA:
		logger_send(LOG_WIFI_MANAGER, __func__, "client data: id %d, data %s", ev, id, data? data : "empty");
		break;
	}
}

RET_CODE wifimgr_change_network(const char* ssid, const char* pass)
{
	RET_CODE result = RETURN_NOK;

	if (ssid && pass)
	{
		if (strlen(ssid) < 32 && strlen(pass) < 64)
		{
			strcpy(wifi_cur_ssid, ssid);
			strcpy(wifi_cur_pass, pass);
			result = RETURN_OK;
		}
	}
	logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "error");
	return result;
}

RET_CODE wifimgr_set_ip_address(const char* address)
{
	RET_CODE result = RETURN_NOK;
	if (address)
	{
		char delims [5] = {"."};
		const char* byte1 = strtok((char*)address, delims);
		const char* byte2 = strtok(NULL, delims);
		const char* byte3 = strtok(NULL, delims);
		const char* byte4 = strtok(NULL, delims);
		wifi_cur_ip_address.ip_address[0] = atoi(byte1);
		wifi_cur_ip_address.ip_address[1] = atoi(byte2);
		wifi_cur_ip_address.ip_address[2] = atoi(byte3);
		wifi_cur_ip_address.ip_address[3] = atoi(byte4);
		result = RETURN_OK;
	}
	logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "error");
	return result;
}

RET_CODE wifimgr_set_server_port(uint16_t port)
{
	RET_CODE result = RETURN_NOK;
	if (port > 999 && port < 10000)
	{
		wifi_server_port = port;
		result = RETURN_OK;
	}
	logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "error");
	return result;
}

RET_CODE wifimgr_send_data(ServerClientID id, const char* data)
{
	return wifi_send_data(id, data, strlen(data));
}

RET_CODE wifimgr_broadcast_data(const char* data)
{
	RET_CODE result = RETURN_NOK;
	for (uint8_t i = 0; i < WIFIMGR_MAX_CLIENTS; i++)
	{
		if (wifi_mgr.clients[i].connected == 1)
		{
			wifi_send_data(wifi_mgr.clients[i].id.id, data, strlen(data));
			result = RETURN_OK;
		}
	}
	return result;
}

RET_CODE wifimgr_get_time(TimeItem* item)
{
	RET_CODE result = RETURN_NOK;
	do
	{
		if (wifi_mgr.server_running)
		{
			if (wifi_close_server() != RETURN_OK)
			{
				logger_send(LOG_ERROR, __func__, "cannot close server");
				break;
			}
		}

		wifi_mgr.server_running = 0;

		if (wifi_allow_multiple_clients(0) != RETURN_NOK)
		{
			logger_send(LOG_ERROR, __func__, "cannot disable multiclient mode");
			break;
		}
		if (wifi_get_time(wifi_ntp_server, item) != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "cannot get ntp time");
			break;
		}

		if (wifi_allow_multiple_clients(1) != RETURN_NOK)
		{
			logger_send(LOG_ERROR, __func__, "cannot enable multiclient mode");
			break;
		}

		if (wifi_open_server(wifi_server_port) != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "cannot open server");
			break;
		}

		wifi_mgr.server_running = 1;
		wifi_mgr.clients_cnt = 0;
		for (uint8_t i = 0; i < WIFIMGR_MAX_CLIENTS; i++)
		{
			wifi_mgr.clients[i].connected = 0;
		}
		result = RETURN_OK;
	}
	while(0);

	if (result != RETURN_OK) wifimgr_reset();
	return result;

}

uint8_t wifimgr_count_clients()
{
	return wifi_mgr.clients_cnt;
}

RET_CODE wifimgr_register_data_callback(void(*callback)(ServerClientID id, const char* data))
{
	RET_CODE result = RETURN_NOK;
	if (!wifimgr_data_callback)
	{
		wifimgr_data_callback = callback;
		result = RETURN_OK;
	}
	return result;
}

void wifimgr_unregister_data_callback(void(*callback)(ServerClientID id, const char* data))
{
	if (wifimgr_data_callback == callback)
	{
		wifimgr_data_callback = NULL;
	}
}

RET_CODE wifimgr_set_max_clients(uint8_t max)
{
	RET_CODE result = RETURN_NOK;
	if (max > 0)
	{
		WIFIMGR_MAX_CLIENTS = max;
		result = RETURN_OK;
	}
	return result;
}
RET_CODE wifimgr_reset()
{
	wifimgr_deinitialize();
	return wifimgr_initialize();
}
void wifimgr_deinitialize()
{
	wifi_reset();
	wifi_deinitialize();
	if (wifi_mgr.clients)
	{
		free(wifi_mgr.clients);
	}
	wifi_mgr.clients_cnt = 0;
	wifi_mgr.wifi_connected = 0;
	wifi_mgr.server_running = 0;

}
