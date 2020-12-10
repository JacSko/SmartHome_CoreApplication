#ifndef _COMMAND_PARSER_H_
#define _COMMAND_PARSER_H_

#include "return_codes.h"
#include "wifi_manager.h"

void cmd_register_bt_sender(RET_CODE(*callback)(const char* data));
void cmd_unregister_bt_sender();

void cmd_register_wifi_sender(RET_CODE(*callback)(ServerClientID id, const char* data));
void cmd_unregister_wifi_sender();

void cmd_handle_wifi_data(ServerClientID id, const char* data);
void cmd_handle_bt_data(const char* data);



#endif
