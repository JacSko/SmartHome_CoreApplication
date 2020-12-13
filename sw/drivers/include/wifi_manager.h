#include "return_codes.h"

#include "wifi_driver.h"
#include "time_counter.h"

/**
 * Initialize WiFi Manager.
 * It starts TCP server which allows clients to connect.
 * Before server is started, time is updated basing on NTP server.
 */
RET_CODE wifimgr_initialize(const WIFI_UART_Config* config);
/**
 * Change the WiFi network to which module is connected.
 * If manager is already running, the connection is established again.
 */
RET_CODE wifimgr_set_network_data(const char* ssid, const char* pass);
/**
 * Change the NTP server data.
 * NTP server may be either IP string or hostname.
 */
RET_CODE wifimgr_set_ntp_server(const char* server);
/**
 * Get currently used NTP server.
 */
const char* wifimgr_get_ntp_server();
/**
 * Get current server port.
 */
uint16_t wifimgr_get_server_port();
/**
 * Change the IP address of device.
 * If manager is already running, the connection is established again.
 */
RET_CODE wifimgr_set_ip_address(const char* address);
/**
 * Change the server port.
 * If manager is already running, the connection is established again.
 */
RET_CODE wifimgr_set_server_port(uint16_t port);
/**
 * Sends data to defined client.
 * It may be useful to send reponse do defined client as a repsonse do CLIENT_DATA event.
 */
RET_CODE wifimgr_send_data(ServerClientID id, const char* data);
/**
 * Sends data to all clients.
 * RETURN_OK is returned if data sent to at least one client.
 */
RET_CODE wifimgr_broadcast_data(const char* data);
/**
 * Gets current time from NTP server
 */
RET_CODE wifimgr_get_time(TimeItem* item);
/**
 * Returns how many clients are currently connected
 */
uint8_t wifimgr_count_clients();
/**
 * Get current IP address of module
 */
RET_CODE wifimgr_get_ip_address(IPAddress* item);
/**
 * Get current network name, to which device is connected
 * Reuqires to pass buffer and size of the buffer.
 */
RET_CODE wifimgr_get_network_name(char* buf, uint8_t buf_size);
/**
 * Get details of all connected clients.
 * It takes pointer to plece, where data is to be stored.
 * Returns count of clients written.
 */
uint8_t wifimgr_get_clients_details(ClientID* buffer);
/**
 * Get maximum number of clients possible.
 */
uint8_t wifi_get_max_clients();
/**
 * Registers data callback client.
 * This callback will be called when connected client sends the data (e.g. command)
 */
RET_CODE wifimgr_register_data_callback(void(*callback)(ServerClientID id, const char* data));
/**
 * Unregister data callback
 */
void wifimgr_unregister_data_callback(void(*callback)(ServerClientID id, const char* data));
/**
 * Resets WiFi driver and Manager.
 */
RET_CODE wifimgr_reset();
/**
 * Deinitialize WiFi Manager.
 */
void wifimgr_deinitialize();

