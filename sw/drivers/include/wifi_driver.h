#ifndef _WIFI_DRIVER_H
#define _WIFI_DRIVER_H
/* ============================= */
/**
 * @file wifi_driver.h
 *
 * @brief WiFi driver to control ESP01 module
 *
 * @details
 * Module is responsible for basic communication with ESP01 module.
 * There are several functions to control device.
 *
 * @author Jacek Skowronek
 * @date 01/12/2020
 */
/* ============================= */
/* =============================
 *  Includes of project headers
 * =============================*/
#include "return_codes.h"
#include "../../time/include/time_counter.h"
/* =============================
 *       Data structures
 * =============================*/
typedef struct
{
	uint32_t baudrate;		/**< Baudrate */
	uint16_t buffer_size;	/**< Maximum size of buffer size */
	uint16_t string_size;	/**< Maximum size of string */
} WIFI_UART_Config;

/** ID used to distinguish connected clients. */
typedef uint8_t ServerClientID;

typedef enum ClientEvent
{
	CLIENT_CONNECTED, 	 /**< Client connects to server */
	CLIENT_DISCONNECTED, /**< Client disconnects from server */
	CLIENT_DATA, 		 /**< Client sends data to server */
} ClientEvent;

/** Represents IP address */
typedef struct IPAddress
{
	uint8_t ip_address [4];
	uint8_t gateway [4];
	uint8_t netmask [4];
} IPAddress;

/** Represents connection type */
typedef enum ConnType
{
	UDP,
	TCP,
	SSL
} ConnType;

/** Used to represents client connected to server */
typedef struct
{
	ServerClientID id;
	ConnType type;
	IPAddress address;
} ClientID;

/**
 * @brief Initialize WiFi module.
 * @param[in] config - Configuration of WiFi module.
 * @return See RETURN_CODES.
 */
RET_CODE wifi_initialize(const WIFI_UART_Config* config);
/**
 * @brief Connect to WiFi network.
 * @param[in] ssid - Name of network.
 * @param[in] password - Password to network.
 * @return See RETURN_CODES.
 */
RET_CODE wifi_connect_to_network(const char* ssid, const char* password);
/**
 * @brief Reset module.
 * @return See RETURN_CODES.
 */
RET_CODE wifi_reset();
/**
 * @brief Connects from WiFi network.
 * @return See RETURN_CODES.
 */
RET_CODE wifi_disconnect_from_network();
/**
 * @brief Set device MAC address.
 * @param[in] mac - New MAC address.
 * @return See RETURN_CODES.
 */
RET_CODE wifi_set_mac_address(const char* mac);
/**
 * @brief Open TCP server.
 * @details To open server, MULTICLIENT mode have to be enabled
 * @param[in] port - server port.
 * @return See RETURN_CODES.
 */
RET_CODE wifi_open_server(uint16_t port);
/**
 * @brief Close TCP server.
 * @return See RETURN_CODES.
 */
RET_CODE wifi_close_server();
/**
 * @brief Enable/Disable MultiClient mode.
 * @param[in] state - Disable(0) or Enable(1).
 * @return See RETURN_CODES.
 */
RET_CODE wifi_allow_multiple_clients(uint8_t state);
/**
 * @brief Send data to connected client.
 * @param[in] id - id of the client.
 * @param[in] data - string to send.
 * @param[in] size - size in bytes of string
 * @return See RETURN_CODES.
 */
RET_CODE wifi_send_data(ServerClientID id, const char* data, uint16_t size);
/**
 * @brief Send data bytes to connected client.
 * @param[in] id - id of the client.
 * @param[in] data - bytes to send.
 * @param[in] size - size of the bytes
 * @return See RETURN_CODES.
 */
RET_CODE wifi_send_bytes(ServerClientID id, const uint8_t* data, uint16_t size);
/**
 * @brief Set IP address of device.
 * @details Driver should be reset to apply setting
 * @param[in] ip_address - new IP address.
 * @return See RETURN_CODES.
 */
RET_CODE wifi_set_ip_address(IPAddress* ip_address);
/**
 * @brief Get current IP address.
 * @param[out] ip_address - place where IP address should be written.
 * @return See RETURN_CODES.
 */
RET_CODE wifi_get_ip_address(IPAddress* ip_address);
/**
 * @brief Read current time from NTP server.
 * @details TCP server have to be stopped to connect NTP server.
 * @param[in] ntp - address of NTP server.
 * @param[out] item - place where time data should be written.
 * @return See RETURN_CODES.
 */
RET_CODE wifi_get_time(const char* ntp_server, TimeItem* item);
/**
 * @brief Get name of the current network
 * @param[out] buffer - place where network name will be written.
 * @param[in] size - size of the passed buffer.
 * @return See RETURN_CODES.
 */
RET_CODE wifi_get_current_network_name(char* buffer, uint8_t size);
/**
 * @brief Get details of connected client.
 * @details Member id of ClientID should be set to desired client before call.
 * @param[out] client - place where client details should be written.
 * @return See RETURN_CODES.
 */
RET_CODE wifi_request_client_details(ClientID* client);
/**
 * @brief Register function which will be called on client event.
 * @details Function will be called on client connected, disconnected or data received.
 * Current implementation allows only to register one callback function.
 * @param[in] callback - Pointer to function.
 * @return See RETURN_CODES.
 */
RET_CODE wifi_register_client_event_callback(void(*callback)(ClientEvent ev, ServerClientID id, const char* data));
/**
 * @brief Unregister callback function.
 * @return None.
 */
void wifi_unregister_client_event_callback();
/**
 * @brief Function to be called in main thread loop.
 * @return None.
 */
void wifi_data_watcher();
/**
 * @brief Deinitialize WiFi module.
 * @return None.
 */
void wifi_deinitialize();




#endif
