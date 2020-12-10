#include <string.h>
#include <stdlib.h>

#include "wifi_driver.h"
#include "uart_engine.h"
#include "string_formatter.h"
#include "Logger.h"

const uint16_t DEFAULT_REPLY_TIMEOUT_MS = 1000;
const uint16_t WIFI_CONNECT_TIMEOUT = 5000;
const uint16_t DHCP_IP_GET_TIMEOUT_MS = 10000;
const uint16_t SERVER_CONN_TIMEOUT_MS = 10000;

char TX_BUFFER[128];
char* RX_BUFFER;

void(*wifi_status_callback)(ClientEvent ev, ServerClientID id, const char* data);
#define TX_BUF_SIZE sizeof(TX_BUFFER)/sizeof(TX_BUFFER[0])

/*	INTERNAL FUNCTIONS  */
RET_CODE wifi_send_command();
RET_CODE wifi_send_and_wait(uint32_t timeout);
RET_CODE wifi_send_and_wait_defined_response(const char* response, uint32_t timeout);
RET_CODE wifi_wait_for_response(uint32_t timeout);
RET_CODE wifi_wait_for_defined_response(const char* resp, uint32_t timeout);
RET_CODE wifi_wait_for_bytes(uint16_t bytes_count, uint32_t timeout);
RET_CODE wifi_test();
RET_CODE wifi_disable_echo();
RET_CODE wifi_connect_server(ConnType type, const char* server, uint16_t port);
RET_CODE wifi_disconnect_server();
void wifi_on_uart_data(const char* data);
void wifi_convert_ntp_time(uint8_t* data, TimeItem* time);


RET_CODE wifi_initialize()
{
	RET_CODE result = RETURN_NOK;

	UART_Config config = {115200, 1024, 512};
	if (uartengine_initialize(&config) == RETURN_OK)
	{
		if (uartengine_register_callback(&wifi_on_uart_data) == RETURN_OK)
		{
			if (wifi_reset() == RETURN_OK)
			{
				if (wifi_disable_echo() == RETURN_OK)
				{
					result = wifi_test();
				}
			}
		}
	}
	return result;
}

RET_CODE wifi_register_client_event_callback(void(*callback)(ClientEvent ev, ServerClientID id, const char* data))
{
	RET_CODE result = RETURN_NOK;
	if (!wifi_status_callback)
	{
		wifi_status_callback = callback;
		result = RETURN_OK;
	}
	return result;
}

void wifi_unregister_client_event_callback()
{
	wifi_status_callback = NULL;
}

void wifi_call_client_callback(ClientEvent ev, ServerClientID id, const char* data)
{
	if (wifi_status_callback)
	{
		wifi_status_callback(ev, id, data);
	}
}

RET_CODE wifi_send_command()
{
	return uartengine_send_string(TX_BUFFER);
}

RET_CODE wifi_send_and_wait(uint32_t timeout)
{
	RET_CODE result = wifi_send_command();
	if (result == RETURN_OK)
	{
		result = wifi_wait_for_response(timeout);
	}
	return result;
}

RET_CODE wifi_send_and_wait_defined_response(const char* response, uint32_t timeout)
{
	if (!response) return RETURN_ERROR;
	RET_CODE result = wifi_send_command();
	if (result == RETURN_OK)
	{
		result = wifi_wait_for_defined_response(response, timeout);
	}
	return result;
}

void wifi_on_uart_data(const char* data)
{
	if (strstr(data, ",CONNECT")) /* new client connected */
	{
		ServerClientID id = atoi(strtok((char*)data, ","));
		wifi_call_client_callback(CLIENT_CONNECTED, id, NULL);

	}
	else if (strstr(data, ",CLOSED")) /* client disconnected */
	{
		ServerClientID id = atoi(strtok((char*)data, ","));
		wifi_call_client_callback(CLIENT_DISCONNECTED, id, NULL);
	}
	else if (strstr(data, "+IPD,")) /* new data received */
	{
		const char delims[3] = ",:";
        const char* command = strtok((char*)data, delims);
        const char* client = strtok(NULL, delims);
        const char* size = strtok(NULL, delims);
        const char* data = strtok(NULL, delims);

        (void) command;
        (void) size;
        wifi_call_client_callback(CLIENT_DATA, (ServerClientID)atoi(client), data);
	}

}

RET_CODE wifi_wait_for_response(uint32_t timeout)
{
	RET_CODE result = RETURN_NOK;
	uint32_t raw_time = time_get()->time_raw;
	while(time_get()->time_raw <= raw_time + timeout)
	{
		if (uartengine_can_read_string() == RETURN_OK)
		{
			RX_BUFFER = (char*) uartengine_get_string();
			result = RETURN_OK;
			break;
		}
	}
	return result;
}

RET_CODE wifi_wait_for_defined_response(const char* resp, uint32_t timeout)
{
	RET_CODE result = RETURN_NOK;
	if (!resp) return RETURN_ERROR;

	uint32_t raw_time = time_get()->time_raw;
	while(time_get()->time_raw <= raw_time + timeout)
	{
		if (uartengine_can_read_string() == RETURN_OK)
		{
			RX_BUFFER = (char*)uartengine_get_string();
			if (!strcmp(resp, RX_BUFFER))
			{
				result = RETURN_OK;
				break;
			}
		}
	}
	return result;
}

RET_CODE wifi_wait_for_bytes(uint16_t bytes_count, uint32_t timeout)
{
	RET_CODE result = RETURN_NOK;
	uint32_t raw_time = time_get()->time_raw;
	while(time_get()->time_raw <= raw_time + timeout)
	{
		if (uartengine_count_bytes() == bytes_count)
		{
			RX_BUFFER = (char*)uartengine_get_bytes();
			result = RETURN_OK;
			break;
		}
	}
	return result;
}

void wifi_deinitialize()
{
	uartengine_deinitialize();
	wifi_status_callback = NULL;

}

/*
 * 		API COMMANDS implementation
 */

RET_CODE wifi_test()
{
	RET_CODE result = RETURN_NOK;
	string_format(TX_BUFFER, "AT\r\n");
	result = wifi_send_and_wait(DEFAULT_REPLY_TIMEOUT_MS);
	if (result == RETURN_OK)
	{
		result = !strcmp(RX_BUFFER, "OK")? RETURN_OK : RETURN_NOK;
	}
	return result;
}

RET_CODE wifi_reset()
{
	RET_CODE result = RETURN_NOK;
	string_format(TX_BUFFER, "AT+RST\r\n");
	if (wifi_send_command() == RETURN_OK)
	{
		time_wait(500);
		uartengine_clear_rx();	/* remove all strange bytes received after reset */
		result = RETURN_OK;
	}
	return result;
}

RET_CODE wifi_disable_echo()
{
	string_format(TX_BUFFER, "ATE0\r\n");
	return wifi_send_and_wait_defined_response("OK", DEFAULT_REPLY_TIMEOUT_MS);
}

RET_CODE wifi_send_data(ServerClientID id, const char* data, uint16_t size)
{
	RET_CODE result = RETURN_NOK;
	string_format(TX_BUFFER, "AT+CIPSEND=%d,%d\r\n", id, size);
	if (wifi_send_and_wait_defined_response("OK", DEFAULT_REPLY_TIMEOUT_MS) == RETURN_OK)
	{
		if (uartengine_send_string(data) == RETURN_OK)
		{
			result = wifi_wait_for_defined_response("SEND OK", DEFAULT_REPLY_TIMEOUT_MS);
		}
	}
	return result;
}

RET_CODE wifi_connect_to_network(const char* ssid, const char* password)
{
	RET_CODE result = RETURN_NOK;
	if (!ssid || !password) return RETURN_ERROR;
	string_format(TX_BUFFER, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", ssid, password);
	if (wifi_send_command() == RETURN_OK)
	{
		if (wifi_wait_for_defined_response("WIFI CONNECTED", WIFI_CONNECT_TIMEOUT) == RETURN_OK)
		{
			if (wifi_wait_for_defined_response("WIFI GOT IP", DHCP_IP_GET_TIMEOUT_MS) == RETURN_OK)
			{
				if (wifi_wait_for_defined_response("OK", WIFI_CONNECT_TIMEOUT) == RETURN_OK)
				{
					result = RETURN_OK;
				}
			}
		}
	}
	return result;
}

RET_CODE wifi_disconnect_from_network()
{
	string_format(TX_BUFFER, "AT+CWQAP\r\n");
	return wifi_send_and_wait_defined_response("OK", DEFAULT_REPLY_TIMEOUT_MS);
}

RET_CODE wifi_set_mac_address(const char* mac)
{
	if (!mac) return RETURN_ERROR;
	string_format(TX_BUFFER, "AT+CIPSTAMAC_CUR=\"%s\"\r\n", mac);
	return wifi_send_and_wait_defined_response("OK", DEFAULT_REPLY_TIMEOUT_MS);
}

RET_CODE wifi_open_server(uint16_t port)
{
	string_format(TX_BUFFER, "AT+CIPSERVER=1,%d\r\n", port);
	return wifi_send_and_wait_defined_response("OK", DEFAULT_REPLY_TIMEOUT_MS);
}
RET_CODE wifi_close_server()
{
	string_format(TX_BUFFER, "AT+CIPSERVER=0\r\n");
	return wifi_send_and_wait_defined_response("OK", DEFAULT_REPLY_TIMEOUT_MS);
}

RET_CODE wifi_allow_multiple_clients(uint8_t state)
{
	string_format(TX_BUFFER, "AT+CIPMUX=%s\r\n", state? "1" : "0");
	return wifi_send_and_wait_defined_response("OK", DEFAULT_REPLY_TIMEOUT_MS);
}

RET_CODE wifi_connect_server(ConnType type, const char* server, uint16_t port)
{
	const char* conn_name = type == UDP? "UDP" : type == TCP? "TCP" : "SSL";

	string_format(TX_BUFFER, "AT+CIPSTART=\"%s\",\"%s\",%d\r\n", conn_name, server, port);
	return wifi_send_and_wait_defined_response("OK", SERVER_CONN_TIMEOUT_MS);
}

RET_CODE wifi_disconnect_server()
{
	string_format(TX_BUFFER, "AT+CIPCLOSE\r\n");
	return wifi_send_and_wait_defined_response("OK", SERVER_CONN_TIMEOUT_MS);
}

RET_CODE wifi_get_time(const char* ntp_server, TimeItem* item)
{
	if (!item || !ntp_server) return RETURN_ERROR;

	RET_CODE result = RETURN_NOK;
	const char ntp_request [] = "c                                               ";	/* 'c' + 47 spaces is the request to ntp server to get time */
	const uint8_t ntp_telegram_size = 48;
	const uint16_t ntp_server_port = 123;
	const uint8_t ipd_header_size = 8;
	const uint8_t ntp_timestamp_offset = 40;

	if (wifi_connect_server(UDP, ntp_server, ntp_server_port) == RETURN_OK)
	{
		string_format(TX_BUFFER, "AT+CIPSEND=%d\r\n",ntp_telegram_size);
		if (wifi_send_and_wait_defined_response("OK", DEFAULT_REPLY_TIMEOUT_MS) == RETURN_OK)
		{
			if (uartengine_send_string(ntp_request) == RETURN_OK)
			{
				if (wifi_wait_for_defined_response("SEND OK", DEFAULT_REPLY_TIMEOUT_MS) == RETURN_OK)
				{
					result = wifi_wait_for_bytes(2 + ipd_header_size + ntp_telegram_size, DEFAULT_REPLY_TIMEOUT_MS); /* there is CRLF sequence at the beginning of data */
					if (result == RETURN_OK)
					{
						wifi_convert_ntp_time((uint8_t*)(RX_BUFFER + 2 + ipd_header_size + ntp_timestamp_offset), item);
					}
				}
			}
		}
		wifi_disconnect_server();
	}
	return result;
}

RET_CODE wifi_set_ip_address(IPAddress* ip_address)
{
	string_format(TX_BUFFER, "AT+CIPSTA_CUR=\"%d.%d.%d.%d\"\r\n", ip_address->ip_address[0], ip_address->ip_address[1],
																  ip_address->ip_address[2], ip_address->ip_address[3]);
	return wifi_send_and_wait_defined_response("OK", DEFAULT_REPLY_TIMEOUT_MS);
}

RET_CODE wifi_get_ip_address(IPAddress* ip_address)
{
	RET_CODE result = RETURN_NOK;

	if (!ip_address) return RETURN_ERROR;

	string_format(TX_BUFFER, "AT+CIPSTA_CUR?\r\n");

	if (wifi_send_command() == RETURN_OK)
	{
		for (uint8_t i = 0; i < 3; i++)
		{
			if (wifi_wait_for_response(DEFAULT_REPLY_TIMEOUT_MS) == RETURN_OK)
			{
				char delims [5] = {":\".,"};
				const char* command = strtok((char*)RX_BUFFER, delims);
				if (!strcmp(command, "+CIPSTA_CUR"))
				{
					const char* type = strtok(NULL, delims);
					const char* byte1 = strtok(NULL, delims);
					const char* byte2 = strtok(NULL, delims);
					const char* byte3 = strtok(NULL, delims);
					const char* byte4 = strtok(NULL, delims);
					if (!strcmp(type, "ip"))
					{
						ip_address->ip_address[0] = atoi(byte1);
						ip_address->ip_address[1] = atoi(byte2);
						ip_address->ip_address[2] = atoi(byte3);
						ip_address->ip_address[3] = atoi(byte4);
					}
					else if (!strcmp(type, "gateway"))
					{
						ip_address->gateway[0] = atoi(byte1);
						ip_address->gateway[1] = atoi(byte2);
						ip_address->gateway[2] = atoi(byte3);
						ip_address->gateway[3] = atoi(byte4);
					}
					else
					{
						ip_address->netmask[0] = atoi(byte1);
						ip_address->netmask[1] = atoi(byte2);
						ip_address->netmask[2] = atoi(byte3);
						ip_address->netmask[3] = atoi(byte4);
					}
					result = RETURN_OK;
				}
				else
				{
					result = RETURN_NOK;
					break;
				}
			}
			else
			{
				result = RETURN_NOK;
				break;
			}
		}
	}

	return result;
}

RET_CODE wifi_get_current_network_name(char* buffer, uint8_t size)
{
	RET_CODE result = RETURN_NOK;

	if (!buffer) return RETURN_ERROR;

	string_format(TX_BUFFER, "AT+CWJAP_CUR?\r\n");

	if (wifi_send_command() == RETURN_OK)
	{
		if (wifi_wait_for_response(DEFAULT_REPLY_TIMEOUT_MS) == RETURN_OK)
		{
			char delims [5] = {":\".,"};
			const char* command = strtok((char*)RX_BUFFER, delims);
			if (!strcmp(command, "+CWJAP_CUR"))
			{
				const char * name = strtok(NULL, delims);
				if (strlen(name) < size)
				{
					string_format(buffer, "%s", name);
					result = RETURN_OK;
				}
			}
		}
	}
	return result;
}

RET_CODE wifi_request_client_details(ClientID* client)
{
	RET_CODE result = RETURN_NOK;

	if (!client) return RETURN_ERROR;

	string_format(TX_BUFFER, "AT+CIPSTATUS\r\n");

	if (wifi_send_command() == RETURN_OK)
	{
		while (wifi_wait_for_response(DEFAULT_REPLY_TIMEOUT_MS) == RETURN_OK)
		{
			if (!strcmp(RX_BUFFER, "OK"))
			{
				break;
			}
			char delims [5] = {":\".,"};
			const char* command = strtok((char*)RX_BUFFER, delims);
			if (!strcmp(command, "+CIPSTATUS"))
			{
				const char * client_id_str = strtok(NULL, delims);
				const char * client_conn_type_str = strtok(NULL, delims);
				const char* byte1 = strtok(NULL, delims);
				const char* byte2 = strtok(NULL, delims);
				const char* byte3 = strtok(NULL, delims);
				const char* byte4 = strtok(NULL, delims);

				if (client->id == atoi(client_id_str))
				{
					client->type = strcmp(client_conn_type_str, "TCP")==0 ? TCP : UDP;
					client->address.ip_address[0] = atoi(byte1);
					client->address.ip_address[1] = atoi(byte2);
					client->address.ip_address[2] = atoi(byte3);
					client->address.ip_address[3] = atoi(byte4);
					result = RETURN_OK;
				}
			}
		}
	}
	return result;
}

void wifi_convert_ntp_time(uint8_t* data, TimeItem* tim)
{

	const uint32_t LEAPOCH       = (946684800LL + 86400*(31+29));
	const uint32_t DAYS_PER_400Y = (365*400 + 97);
	const uint32_t DAYS_PER_100Y = (365*100 + 24);
	const uint32_t DAYS_PER_4Y   = (365*4   + 1);
	const uint32_t NTP_TO_UNIX_TS = 2208988800;
    uint32_t unix_ts = 0;
    uint32_t days, secs;
    int remdays, remsecs, remyears;
	int qc_cycles, c_cycles, q_cycles;
	int years, months;
	int wday, yday, leap;
	static const char days_in_month[] = {31,30,31,30,31,31,30,31,30,31,31,29};


    unix_ts =  0xFF000000 & (data[0] << 24);
    unix_ts |= 0x00FF0000 & (data[1] << 16);
    unix_ts |= 0x0000FF00 & (data[2] << 8);
    unix_ts |= 0x000000FF & (data[3] << 0);

    unix_ts = unix_ts - NTP_TO_UNIX_TS - LEAPOCH;

    secs = unix_ts;
	days = secs / 86400;
	remsecs = secs % 86400;
	if (remsecs < 0) {
		remsecs += 86400;
		days--;
	}

    wday = (3+days)%7;
	if (wday < 0) wday += 7;

	qc_cycles = days / DAYS_PER_400Y;
	remdays = days % DAYS_PER_400Y;
	if (remdays < 0) {
		remdays += DAYS_PER_400Y;
		qc_cycles--;
	}

	c_cycles = remdays / DAYS_PER_100Y;
	if (c_cycles == 4) c_cycles--;
	remdays -= c_cycles * DAYS_PER_100Y;

	q_cycles = remdays / DAYS_PER_4Y;
	if (q_cycles == 25) q_cycles--;
	remdays -= q_cycles * DAYS_PER_4Y;

	remyears = remdays / 365;
	if (remyears == 4) remyears--;
	remdays -= remyears * 365;

	leap = !remyears && (q_cycles || !c_cycles);
	yday = remdays + 31 + 28 + leap;
	if (yday >= 365+leap) yday -= 365+leap;

	years = remyears + 4*q_cycles + 100*c_cycles + 400*qc_cycles;

	for (months=0; days_in_month[months] <= remdays; months++)
		remdays -= days_in_month[months];

	tim->year = years + 100;
	tim->month = months + 2;
	if (tim->month >= 12) {
		tim->month -=12;
		tim->year++;
	}
	tim->year+=1900;
	tim->month++;
	tim->day = remdays + 1;

	tim->hour = remsecs / 3600;
	tim->minute = remsecs / 60 % 60;
	tim->second = remsecs % 60;
}

