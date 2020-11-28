#include <string.h>

#include "wifi_driver.h"
#include "uart_engine.h"
#include "string_formatter.h"
#include "../../time/include/time_counter.h"

const uint16_t DEFAULT_REPLY_TIMEOUT_MS = 1000;
const uint16_t WIFI_CONNECT_TIMEOUT = 5000;
const uint16_t DHCP_IP_GET_TIMEOUT_MS = 10000;
const uint16_t SERVER_CONN_TIMEOUT_MS = 10000;

char TX_BUFFER[128];
const char* RX_BUFFER;

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
			if (wifi_disable_echo() == RETURN_OK)
			{
				result = wifi_test();
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

}

RET_CODE wifi_wait_for_response(uint32_t timeout)
{
	RET_CODE result = RETURN_NOK;
	uint32_t raw_time = time_get()->time_raw;
	while(time_get()->time_raw <= raw_time + timeout)
	{
		if (uartengine_can_read_string() == RETURN_OK)
		{
			RX_BUFFER = uartengine_get_string();
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
			RX_BUFFER = uartengine_get_string();
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
			RX_BUFFER = (char*) uartengine_get_bytes();
			result = RETURN_OK;
			break;
		}
	}
	return result;
}

void wifi_deinitialize()
{
	uartengine_deinitialize();
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

RET_CODE wifi_disable_echo()
{
	string_format(TX_BUFFER, "ATE0\r\n");
	return wifi_send_and_wait_defined_response("OK", DEFAULT_REPLY_TIMEOUT_MS);
}

RET_CODE wifi_enable_echo()
{
	string_format(TX_BUFFER, "ATE1\r\n");
	return wifi_send_and_wait_defined_response("OK", DEFAULT_REPLY_TIMEOUT_MS);
}

RET_CODE wifi_send_data(ServerClientID id, const char* data, uint16_t size)
{
	RET_CODE result = RETURN_NOK;
	string_format(TX_BUFFER, "AT+CIPSEND=%d,%d\r\n", id, size);
	if (wifi_send_and_wait_defined_response("> ", DEFAULT_REPLY_TIMEOUT_MS) == RETURN_OK)
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

RET_CODE wifi_open_udp_server(uint16_t port)
{
	string_format(TX_BUFFER, "AT+CIPSERVER=1,%d\r\n", port);
	return wifi_send_and_wait_defined_response("OK", DEFAULT_REPLY_TIMEOUT_MS);
}
RET_CODE wifi_close_udp_server()
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
	if (!item) return RETURN_ERROR;

	RET_CODE result = RETURN_NOK;
	const char ntp_request [] = "c                                               ";	/* 'c' + 47 spaces is the request to ntp server to get time */
	const uint8_t ntp_telegram_size = 48;
	const uint8_t ipd_header_size = 8;
	const uint8_t ntp_timestamp_offset = 39;

	string_format(TX_BUFFER, "AT+CIPSEND=%d\r\n",ntp_telegram_size);
	if (wifi_send_and_wait_defined_response("> ", DEFAULT_REPLY_TIMEOUT_MS) == RETURN_OK)
	{
		if (uartengine_send_string(ntp_request) == RETURN_OK)
		{
			if (wifi_wait_for_defined_response("SEND OK", DEFAULT_REPLY_TIMEOUT_MS) == RETURN_OK)
			{
				result = wifi_wait_for_bytes(ntp_telegram_size + ipd_header_size, DEFAULT_REPLY_TIMEOUT_MS);
				if (result == RETURN_OK)
				{
					wifi_convert_ntp_time((uint8_t*)(RX_BUFFER + ipd_header_size + ntp_timestamp_offset), item);
				}
			}
		}
	}
	return result;
}

RET_CODE wifi_set_ip_address(IPAddress* ip_address)
{
	string_format(TX_BUFFER, "AT+CIPSTA_CUR=\"%d.%d.%d.%d\"\r\n", ip_address->ip_address[0], ip_address->ip_address[1],
																  ip_address->ip_address[2], ip_address->ip_address[3]);
	return wifi_send_and_wait_defined_response("OK", DEFAULT_REPLY_TIMEOUT_MS);
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

