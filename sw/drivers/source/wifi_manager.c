#include "wifi_manager.h"
#include "Logger.h"
#include <stdlib.h>
#include <string.h>

void wifimgr_on_client_event(ClientEvent ev, ServerClientID id, const char* data);
void(*wifimgr_data_callback)(ServerClientID id, const char* data);

uint8_t 	WIFIMGR_MAX_CLIENTS = 2;
char 		WIFIMGR_NETWORK_SSID [32] = "NIE_KRADNIJ_INTERNETU!!!";
char 		WIFIMGR_NETWORK_PASS [64] = "radionet0098";
char 		WIFIMGR_NTP_SERVER [100] = "194.146.251.101";
IPAddress 	WIFIMGR_IP_ADDRESS = {{192,168,100,100},{0,0,0,0},{0,0,0,0}};
uint16_t 	WIFIMGR_SERVER_PORT = 4444;

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
	char* rx_buffer;
	WIFI_UART_Config config;
	char* ssid;
	char* pass;
} WifiMgr;


WifiMgr wifi_mgr;


RET_CODE wifimgr_initialize(const WIFI_UART_Config* config)
{
	logger_send(LOG_WIFI_MANAGER, __func__, "initializing wifimgr");
	wifi_mgr.ssid = WIFIMGR_NETWORK_SSID;
	wifi_mgr.pass = WIFIMGR_NETWORK_PASS;

	wifi_mgr.clients_cnt = 0;
	wifi_mgr.wifi_connected = 0;
	wifi_mgr.server_port = WIFIMGR_SERVER_PORT;
	wifi_mgr.server_running = 0;
	wifi_mgr.config = *config;

	RET_CODE result = RETURN_NOK;

	do
	{
		if (wifi_initialize(config) != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "Cannot init wifi driver");
			break;
		}

		if (wifi_set_ip_address(&WIFIMGR_IP_ADDRESS) != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "Cannot set ip address");
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

		RET_CODE time_ok = wifi_get_time(WIFIMGR_NTP_SERVER, &tim);
		if (time_ok != RETURN_OK)
		{
			/* workaround - due to bug in ESP, after initializiation there is not first response from NTP server */
			logger_send(LOG_ERROR, __func__, "Cannot get NTP time, trying again!");
			time_ok = wifi_get_time(WIFIMGR_NTP_SERVER, &tim);
		}
		if (time_ok == RETURN_OK)
		{
			time_set_utc(&tim);
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
		wifi_mgr.rx_buffer = (char*) malloc (sizeof(char) * wifi_mgr.config.string_size);
		if (wifi_mgr.rx_buffer && wifi_mgr.clients)
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
			wifi_mgr.clients[id].connected = 1;
			ClientID client_data = {};
			client_data.id = id;
			if (wifi_request_client_details(&client_data) == RETURN_OK)
			{
				logger_send(LOG_WIFI_MANAGER, __func__, "client: id %d, ip: %d.%d.%d.%d", client_data.id, client_data.address.ip_address[0], client_data.address.ip_address[1],
																								 client_data.address.ip_address[2], client_data.address.ip_address[3]);
			}
			else
			{
				logger_send(LOG_ERROR, __func__, "Cannot get client details");
			}
			wifi_mgr.clients[id].id = client_data;
		}
		break;
	case CLIENT_DISCONNECTED:
		wifi_mgr.clients_cnt--;
		wifi_mgr.clients[id].connected = 0;
		logger_send(LOG_WIFI_MANAGER, __func__, "Client %d disconnected", id);
		break;
	case CLIENT_DATA:
		if (wifi_mgr.clients[id].connected)
		{
			if (wifi_mgr.config.string_size >= strlen(data))
			{
				if (wifimgr_data_callback)
				{
					strcpy(wifi_mgr.rx_buffer, data);
					wifimgr_data_callback(id, wifi_mgr.rx_buffer);
				}
			}
			else
			{
				logger_send(LOG_ERROR, __func__, "Buffer too small %d/%d", strlen(data), wifi_mgr.config.string_size);
			}
		}
		else
		{
			logger_send(LOG_ERROR, __func__, "Received data from not connected client %d", id);
		}
		break;
	}
}

RET_CODE wifimgr_set_network_data(const char* ssid, const char* pass)
{
	RET_CODE result = RETURN_NOK;

	if (ssid && pass)
	{
		if (strlen(ssid) < 32 && strlen(pass) < 64)
		{
			strcpy(WIFIMGR_NETWORK_SSID, ssid);
			strcpy(WIFIMGR_NETWORK_PASS, pass);
			result = RETURN_OK;
		}
	}
	logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "error");
	return result;
}

RET_CODE wifimgr_set_ntp_server(const char* server)
{
	RET_CODE result = RETURN_NOK;

	if (server)
	{
		strcpy(WIFIMGR_NTP_SERVER, server);
		result = RETURN_OK;
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

		if (byte1 && byte2 && byte3 && byte4)
		{
			WIFIMGR_IP_ADDRESS.ip_address[0] = atoi(byte1);
			WIFIMGR_IP_ADDRESS.ip_address[1] = atoi(byte2);
			WIFIMGR_IP_ADDRESS.ip_address[2] = atoi(byte3);
			WIFIMGR_IP_ADDRESS.ip_address[3] = atoi(byte4);
			result = RETURN_OK;
		}

	}
	logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "error");
	return result;
}

RET_CODE wifimgr_set_server_port(uint16_t port)
{
	RET_CODE result = RETURN_NOK;
	if (port > 999 && port < 10000)
	{
		WIFIMGR_SERVER_PORT = port;
		result = RETURN_OK;
	}
	logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "error");
	return result;
}
const char* wifimgr_get_ntp_server()
{
	return WIFIMGR_NTP_SERVER;
}
uint16_t wifimgr_get_server_port()
{
	return wifi_mgr.server_port;
}

RET_CODE wifimgr_send_data(ServerClientID id, const char* data)
{
	RET_CODE result = RETURN_NOK;

	if (wifi_mgr.clients[id].connected)
	{
		result = wifi_send_data(id, data, strlen(data));
	}
	return result;
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

		if (wifi_allow_multiple_clients(0) != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "cannot disable multiclient mode");
			break;
		}
		if (wifi_get_time(WIFIMGR_NTP_SERVER, item) != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "cannot get ntp time");
			break;
		}

		if (wifi_allow_multiple_clients(1) != RETURN_OK)
		{
			logger_send(LOG_ERROR, __func__, "cannot enable multiclient mode");
			break;
		}

		if (wifi_open_server(WIFIMGR_SERVER_PORT) != RETURN_OK)
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

	if (result != RETURN_OK)
	{
		wifimgr_reset();
	}
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
RET_CODE wifimgr_get_ip_address(IPAddress* item)
{
	return wifi_get_ip_address(item);
}

RET_CODE wifimgr_get_network_name(char* buf, uint8_t buf_size)
{
	return wifi_get_current_network_name(buf, buf_size);
}

uint8_t wifimgr_get_clients_details(ClientID* buffer)
{
	uint8_t result = 0;
	for (uint8_t i = 0; i < WIFIMGR_MAX_CLIENTS; i++)
	{
		if (wifi_mgr.clients[i].connected)
		{
			*buffer = wifi_mgr.clients[i].id;
			buffer++;
			result++;
		}
	}
	return result;
}
uint8_t wifi_get_max_clients()
{
	return WIFIMGR_MAX_CLIENTS;
}
RET_CODE wifimgr_reset()
{
	wifimgr_deinitialize();
	return wifimgr_initialize(&wifi_mgr.config);
}
void wifimgr_deinitialize()
{
	wifi_reset();
	wifi_deinitialize();
	if (wifi_mgr.clients)
	{
		free(wifi_mgr.clients);
		free(wifi_mgr.rx_buffer);
		wifi_mgr.clients = NULL;
		wifi_mgr.rx_buffer = NULL;
	}
	wifi_mgr.clients_cnt = 0;
	wifi_mgr.wifi_connected = 0;
	wifi_mgr.server_running = 0;

}
