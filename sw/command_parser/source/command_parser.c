/* =============================
 *   Includes of common headers
 * =============================*/
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "command_parser.h"
#include "string_formatter.h"
/* =============================
 *          Defines
 * =============================*/
#define CMD_REPLY_BUFFER_SIZE 512
#define CMD_PARSER_USE_ECHO 1
/* =============================
 *   Internal module functions
 * =============================*/
void cmd_prepare_response(RET_CODE result);
void cmd_send_response();
RET_CODE cmd_parse_data(const char* data);
RET_CODE cmd_handle_wifimgr_subcommand(const char** command, uint8_t size);
RET_CODE cmd_handle_slm_subcommand(const char** command, uint8_t size);
/* =============================
 *      Module variables
 * =============================*/
const uint8_t CMD_UNKNOWN_ID = 255;
char CMD_REPLY_BUFFER [CMD_REPLY_BUFFER_SIZE];
RET_CODE(*BT_SEND)(const char* data);
RET_CODE(*WIFI_SEND)(ServerClientID id, const char* data);
ServerClientID CURR_ID;


void cmd_register_bt_sender(RET_CODE(*callback)(const char* data))
{
	BT_SEND = callback;
}
void cmd_unregister_bt_sender()
{
	BT_SEND = NULL;
}

void cmd_register_wifi_sender(RET_CODE(*callback)(ServerClientID id, const char* data))
{
	WIFI_SEND = callback;
}
void cmd_unregister_wifi_sender()
{
	WIFI_SEND = NULL;
}

void cmd_handle_wifi_data(ServerClientID id, const char* data)
{
	CURR_ID = id;
	if (CMD_PARSER_USE_ECHO)
	{
		/* send echo */
		string_format(CMD_REPLY_BUFFER, "CMD: %s\n", data);
		cmd_send_response();
	}
	/* send response */
	cmd_prepare_response(cmd_parse_data(data));
	cmd_send_response();
}
void cmd_handle_bt_data(const char* data)
{
	CURR_ID = CMD_UNKNOWN_ID;
	if (CMD_PARSER_USE_ECHO)
	{
		/* send echo */
		string_format(CMD_REPLY_BUFFER, "CMD: %s\n", data);
		cmd_send_response();
	}
	/* send response */
	cmd_prepare_response(cmd_parse_data(data));
	cmd_send_response();
}

RET_CODE cmd_parse_data(const char* data)
{
	RET_CODE result = RETURN_NOK;
	char cmd_received [strlen(data)];
	const char* cmd_items [10];
	char delims[3] = " ";
	uint8_t index = 0;
	string_format(cmd_received, "%s", data);
	char* item = strtok(cmd_received, delims);

	while(item)
	{
		cmd_items[index] = item;
		item = strtok(NULL, delims);
		index++;
	}

	if (!strcmp(cmd_items[0], "wifimgr")) result =  cmd_handle_wifimgr_subcommand(cmd_items, index);
	if (!strcmp(cmd_items[0], "slm")) result =  cmd_handle_slm_subcommand(cmd_items, index);

	return result;
}

RET_CODE cmd_handle_slm_subcommand(const char** command, uint8_t size)
{
   RET_CODE result = RETURN_NOK;

   return result;
}

RET_CODE cmd_handle_wifimgr_subcommand(const char** command, uint8_t size)
{
	RET_CODE result = RETURN_NOK;
	if (!strcmp(command[1], "reset"))
	{
		result = wifimgr_reset();
	}
	else if (!strcmp(command[1], "ip"))
	{
		if(!strcmp(command[2], "set"))
		{
			result = wifimgr_set_ip_address(command[3]);
		}
		else if (!strcmp(command[2], "get"))
		{
			IPAddress item = {};
			result = wifimgr_get_ip_address(&item);
			string_format(CMD_REPLY_BUFFER, "IP:%d.%d.%d.%d\n", item.ip_address[0], item.ip_address[1], item.ip_address[2], item.ip_address[3]);
			cmd_send_response();
		}
	}
	else if (!strcmp(command[1], "network"))
	{
		if(!strcmp(command[2], "set"))
		{
			result = wifimgr_set_network_data(command[3], command[4]);
		}
		else if (!strcmp(command[2], "get"))
		{
			char network_name [100];
			result = wifimgr_get_network_name(network_name, 100);
			string_format(CMD_REPLY_BUFFER, "SSID:%s\n", network_name);
			cmd_send_response();
		}
	}
	else if (!strcmp(command[1], "ntpserver"))
	{
		if(!strcmp(command[2], "set"))
		{
			result = wifimgr_set_ntp_server(command[3]);
		}
		else if (!strcmp(command[2], "get"))
		{
			const char* server = wifimgr_get_ntp_server();
			result = server? RETURN_OK : RETURN_NOK;
			string_format(CMD_REPLY_BUFFER, "NTP:%s\n", server);
			cmd_send_response();
		}
	}
	else if (!strcmp(command[1], "serverport"))
	{
		if(!strcmp(command[2], "set"))
		{
			result = wifimgr_set_server_port(atoi(command[3]));
		}
		else if (!strcmp(command[2], "get"))
		{
			uint16_t port = wifimgr_get_server_port();
			result = RETURN_OK;
			string_format(CMD_REPLY_BUFFER, "PORT:%d\n", port);
			cmd_send_response();
		}
	}
	else if (!strcmp(command[1], "clients"))
	{
		if(!strcmp(command[2], "get"))
		{
			ClientID buffer[wifi_get_max_clients()];
			uint8_t count = wifimgr_get_clients_details(buffer);
			result = count != 0? RETURN_OK : RETURN_NOK;
			for (uint8_t i = 0; i < count; i++)
			{
				string_format(CMD_REPLY_BUFFER, "CLIENT%d: type %d, %d.%d.%d.%d\n", buffer[i].id,
											buffer[i].type, buffer[i].address.ip_address[0],
															buffer[i].address.ip_address[1],
															buffer[i].address.ip_address[2],
															buffer[i].address.ip_address[3]);
				cmd_send_response();
			}

		}
	}
	return result;
}

void cmd_prepare_response(RET_CODE result)
{
	string_format(CMD_REPLY_BUFFER, "%s", result == RETURN_OK? "OK\n" : "ERROR\n");
}

void cmd_send_response()
{
	CURR_ID != CMD_UNKNOWN_ID? WIFI_SEND(CURR_ID, CMD_REPLY_BUFFER) : BT_SEND(CMD_REPLY_BUFFER);
}
