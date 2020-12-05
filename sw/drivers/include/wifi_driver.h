#ifndef _WIFI_DRIVER_H
#define _WIFI_DRIVER_H

#include "return_codes.h"
#include "stm32f4xx.h"
#include "../../time/include/time_counter.h"


/** ID used to distinguish connected clients. */
typedef uint8_t ServerClientID;

typedef enum ClientEvent
{
	CLIENT_CONNECTED, /**< Client connects to server */
	CLIENT_DISCONNECTED, /**< Client disconnects from server */
	CLIENT_DATA, /**< Client sends data to server */
} ClientEvent;

/** Represents IP address */
typedef struct IPAddress
{
	uint8_t ip_address [4];
	uint8_t gateway [4];
	uint8_t netmask [4];
} IPAddress;

/**< Represents connection type */
typedef enum ConnType
{
	UDP,
	TCP,
	SSL
} ConnType;

/**< Used to represents client connected to server */
typedef struct
{
	ServerClientID id;
	ConnType type;
	IPAddress address;
} ClientID;

/**
 * Initialization of WiFi module.
 * This function performs communication check with external chip.
 * Returns RETURN_OK if UART driver has been initialized
 * and communication with ESP module is working.
 */
RET_CODE wifi_initialize();

/**
 * Setting up a connection with WiFi network
 */
RET_CODE wifi_connect_to_network(const char* ssid, const char* password);

/**
 * Resets WiFi module
 */
RET_CODE wifi_reset();

/**
 * Disconnects from WiFi network
 */
RET_CODE wifi_disconnect_from_network();

/**
 * Set ESP device MAC address.
 * Note: MAC address set should be done before connecting to WiFi.
 */
RET_CODE wifi_set_mac_address(const char* mac);

/**
 * Opens TCP server on defined port.
 * Note: To open server, ESP device shall be configured to
 * 		 allow multiple client connections.
 */
RET_CODE wifi_open_server(uint16_t port);

/**
 * Close TCP server.
 */
RET_CODE wifi_close_server();

/**
 * Allows multiple connections to server.
 * Note: Multiple connections should be allowed only in server mode.
 * 		 If server is not running, should be set to False.
 */
RET_CODE wifi_allow_multiple_clients(uint8_t state);

/**
 * Sends data to defined client.
 * It is important, that data size is correct
 */
RET_CODE wifi_send_data(ServerClientID id, const char* data, uint16_t size);

/**
 * Sets ESP IP address.
 * Note: IP address set should be done before connecting to WiFi.
 */
RET_CODE wifi_set_ip_address(IPAddress* ip_address);

/**
 * Returns current IP address.
 */
RET_CODE wifi_get_ip_address(IPAddress* ip_address);

/**
 * Unregister client event callback.
 */
RET_CODE wifi_get_time(const char* ntp_server, TimeItem* item);

/**
 *	Returns current WiFi network name to which device is connected.
 *	Note: If network name is longer than size, the result is not written to buffer.
 */
RET_CODE wifi_get_current_network_name(char* buffer, uint8_t size);

/**
 * Allows to get client information, like IP address and connection type.
 */
RET_CODE wifi_request_client_details(ClientID* client);

/**
 * Register client event callback.
 * Only one callback can be registered.
 */
RET_CODE wifi_register_client_event_callback(void(*callback)(ClientEvent ev, ServerClientID id, const char* data));

/**
 * Unregister client event callback.
 */
void wifi_unregister_client_event_callback();

/**
 * WiFi driver deinitialization.
 */
void wifi_deinitialize();




#endif
