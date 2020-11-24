#ifndef _WIFI_DRIVER_H
#define _WIFI_DRIVER_H

#include <stdint.h>

#include "return_codes.h"


RET_CODE wifi_initialize();
RET_CODE wifi_connect_to_network(const char* ssid, const char* password);
RET_CODE wifi_disconnect_from_network();
RET_CODE wifi_set_mac_address(const char* mac);
RET_CODE wifi_set_max_server_clients(uint8_t count);
RET_CODE wifi_open_udp_server(uint16_t port);
RET_CODE wifi_close_udp_server();


RET_CODE wifi_register_data_callback(void(*callback)(const char* data));
void wifi_unregister_data_callback();

void wifi_deinitialize();




#endif
