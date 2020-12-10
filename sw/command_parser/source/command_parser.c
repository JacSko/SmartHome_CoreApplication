#include <stddef.h>
#include "command_parser.h"
#include "string_formatter.h"

#define CMD_REPLY_BUFFER_SIZE 512


const uint8_t CMD_UNKNOWN_ID = 255;

char CMD_REPLY_BUFFER [CMD_REPLY_BUFFER_SIZE];
RET_CODE(*BT_SEND)(const char* data);
RET_CODE(*WIFI_SEND)(ServerClientID id, const char* data);

void cmd_parse_data(const char* data);
void cmd_send_response(ServerClientID);

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
	cmd_parse_data(data);
	cmd_send_response(id);
}
void cmd_handle_bt_data(const char* data)
{
	cmd_parse_data(data);
	cmd_send_response(CMD_UNKNOWN_ID);
}

void cmd_parse_data(const char* data)
{
	string_format(CMD_REPLY_BUFFER, "CMD: %s\n", data);
}

void cmd_send_response(ServerClientID id)
{
	id != CMD_UNKNOWN_ID? WIFI_SEND(id, CMD_REPLY_BUFFER) : BT_SEND(CMD_REPLY_BUFFER);
}
