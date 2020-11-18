#ifndef _WIFI_DRIVER_H
#define _WIFI_DRIVER_H

#include <stdint.h>

#include "return_codes.h"


void wifi_initialize();

void wifi_register_response_callback(void(*callback)(const char*));
void wifi_unregister_response_callback();

RET_CODE wifi_connect_to_network(const char* ssid, const char* password);
RET_CODE wifi_disconnect_from_network();

RET_CODE wifi_open_server(uint16_t port);
RET_CODE wifi_send_data(const char* data);

void wifi_deinitialize();




#endif
