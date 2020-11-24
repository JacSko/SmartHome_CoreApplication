#include <string.h>

#include "wifi_driver.h"
#include "uart_engine.h"
#include "string_formatter.h"
#include "../../time/include/time_counter.h"

const uint16_t DEFAULT_REPLY_TIMEOUT_MS = 1000;
const uint16_t DHCP_IP_GET_TIMEOUT_MS = 10000;

char TX_BUFFER[128];
const char* RX_BUFFER;

void(*wifi_callback)(const char* data);
#define TX_BUF_SIZE sizeof(TX_BUFFER)/sizeof(TX_BUFFER[0])

/*	INTERNAL FUNCTIONS  */
RET_CODE wifi_send_command();
RET_CODE wifi_send_and_wait(uint32_t timeout);
RET_CODE wifi_wait_for_response(uint32_t timeout);
RET_CODE wifi_wait_for_defined_response(const char* resp, uint32_t timeout);
RET_CODE wifi_test();
void wifi_on_uart_data(const char* data);


RET_CODE wifi_initialize()
{
	RET_CODE result = RETURN_NOK;

	UART_Config config = {115200, '\n', 1024, 512};
	if (uartengine_initialize(&config) == RETURN_OK)
	{
		if (uartengine_register_callback(&wifi_on_uart_data) == RETURN_OK)
		{
			result = wifi_test();
		}
	}

	return result;
}

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

RET_CODE wifi_register_data_callback(void(*callback)(const char* data))
{
	RET_CODE result = RETURN_NOK;
	if (!wifi_callback)
	{
		wifi_callback = callback;
		result = RETURN_OK;
	}
	return result;
}

void wifi_unregister_data_callback()
{
	wifi_callback = NULL;
}

void wifi_on_uart_data(const char* data)
{

}

RET_CODE wifi_wait_for_response(uint32_t timeout)
{
	RET_CODE result = RETURN_NOK;
	TimeItem* t = time_get();
	while(time_get()->time_raw <= t->time_raw + timeout)
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
	TimeItem* t = time_get();
	if (!resp) return RETURN_ERROR;

	while(time_get()->time_raw <= t->time_raw + timeout)
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

void wifi_deinitialize()
{
	uartengine_deinitialize();
}

RET_CODE wifi_connect_to_network(const char* ssid, const char* password)
{
	RET_CODE result = RETURN_NOK;
	if (!ssid || !password) return RETURN_ERROR;
	string_format(TX_BUFFER, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", ssid, password);
	if (wifi_send_command() == RETURN_OK)
	{
		/* wait for response WIFI CONNECTED*/
		if (wifi_wait_for_defined_response("WIFI CONNECTED", DEFAULT_REPLY_TIMEOUT_MS) == RETURN_OK)
		{
			if (wifi_wait_for_defined_response("WIFI GOT IP", DHCP_IP_GET_TIMEOUT_MS) == RETURN_OK)
			{
				if (wifi_wait_for_defined_response("OK", DEFAULT_REPLY_TIMEOUT_MS) == RETURN_OK)
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

RET_CODE wifi_set_max_server_clients(uint8_t count)
{
	string_format(TX_BUFFER, "AT+CIPSERVERMAXCONN=%d\r\n", count);
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

