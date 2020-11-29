#ifndef _WIFI_DRIVER_H
#define _WIFI_DRIVER_H

#include <stdint.h>

#include "return_codes.h"

typedef uint8_t ServerClientID;
typedef enum ClientEvent
{
	CLIENT_CONNECTED,
	CLIENT_DISCONNECTED,
	CLIENT_DATA,
} ClientEvent;


typedef struct IPAddress
{
	uint8_t ip_address [4];
	uint8_t gateway [4];
	uint8_t netmask [4];
} IPAddress;

typedef enum ConnType
{
	UDP,
	TCP,
	SSL
} ConnType;

typedef struct
{
	ServerClientID id;
	ConnType type;
	IPAddress address;
} ClientID;


RET_CODE wifi_initialize();
RET_CODE wifi_connect_to_network(const char* ssid, const char* password);
RET_CODE wifi_disconnect_from_network();
RET_CODE wifi_set_mac_address(const char* mac);
RET_CODE wifi_open_udp_server(uint16_t port);
RET_CODE wifi_close_udp_server();
RET_CODE wifi_allow_multiple_clients(uint8_t state);
RET_CODE wifi_send_data(ServerClientID id, const char* data, uint16_t size);
RET_CODE wifi_set_ip_address(IPAddress* ip_address);
RET_CODE wifi_get_ip_address(IPAddress* ip_address);
RET_CODE wifi_get_current_network_name(char* buffer, uint8_t size);
RET_CODE wifi_request_client_details(ClientID* client);

RET_CODE wifi_register_client_event_callback(void(*callback)(ClientEvent ev, ServerClientID id, const char* data));
void wifi_unregister_client_event_callback();

void wifi_deinitialize();




#endif
