/* ============================= */
/**
 * @file wifi_manager.h
 *
 * @brief Module allows to control WiFi module.
 *
 * @details
 * To control WiFi module, the WiFi manager is using WiFi Driver module.
 * Before module initialization, the respective functions can be called
 * to set IP address, server port and so on.
 * There are default settings that are used when other settings not provided.
 *
 * @author Jacek Skowronek
 * @date 01/11/2020
 */
/* ============================= */

/* =============================
 *  Includes of project headers
 * =============================*/
#include "return_codes.h"
#include "wifi_driver.h"
#include "time_counter.h"

/**
 * @brief Initialize WiFi Manager.
 * @details
 * As the result of initialization, the TCP server is started.
 * Before, the communication WiFi module is checked and the time from NTP server is read.
 * Before initialize, the another methods may be called to set desired SSID, NTP server and so on.
 * @param[in] config - pointer to configuration
 * @return See RETURN_CODES.
 */
RET_CODE wifimgr_initialize(const WIFI_UART_Config* config);
/**
 * @brief Allow to set SSID and PASS of the network which WiFi is connecting to.
 * @details
 * The SSID and PASS cannot be NULL. <br>
 * Maximum length of SSID - 32 bytes, PASS - 64bytes.
 * @param[in] ssid - name of the network
 * @param[in] pass - password to network
 * @return See RETURN_CODES.
 */
RET_CODE wifimgr_set_network_data(const char* ssid, const char* pass);
/**
 * @brief Allow to set NTP server.
 * @details
 * NTP server may be provided using hostname or direct IP address.
 * The NTP cannot be NULL. <br>
 * Maximum length of NTP server - 32 bytes.
 * @param[in] server - NTP server name
 * @return See RETURN_CODES.
 */
RET_CODE wifimgr_set_ntp_server(const char* server);
/**
 * @brief Allow to get NTP server.
 * @return Name of NTP server.
 */
const char* wifimgr_get_ntp_server();
/**
 * @brief Allow to get TCP server port.
 * @return Port.
 */
uint16_t wifimgr_get_server_port();
/**
 * @brief Allow to set IP address of the device.
 * @details
 * IP address should be set before calling initialization.
 * If called later, the module should be reset to make change happen.
 * @param[in] address - IP address in string form (e.g. 127.0.0.1).
 * @return See RETURN_CODES.
 */
RET_CODE wifimgr_set_ip_address(const char* address);
/**
 * @brief Allow to set TCP server port.
 * @details
 * Range: 1000-9999.<br>
 * Server port should be set before calling initialization.
 * If called later, the module should be reset to make change happen.
 * @param[in] port - server port.
 * @return See RETURN_CODES.
 */
RET_CODE wifimgr_set_server_port(uint16_t port);
/**
 * @brief Send data to defined client.
 * @details
 * Data cannot be NULL.<br>
 * @param[in] id - client id
 * @param[in] data - data to send.
 * @return See RETURN_CODES.
 */
RET_CODE wifimgr_send_data(ServerClientID id, const char* data);
/**
 * @brief Send data to all connected clients.
 * @details
 * Data cannot be NULL.<br>
 * @param[in] port - server port.
 * @return See RETURN_CODES.
 */
RET_CODE wifimgr_broadcast_data(const char* data);
/**
 * @brief Read time from NTP server.
 * @details
 * If there is server running, all clients are going to be disconnected.
 * The server is restarted after time get.
 * @param[out] item - place where received time data will be written.
 * @return See RETURN_CODES.
 */
RET_CODE wifimgr_get_time(TimeItem* item);
/**
 * @brief Checks how many clients are connected.
 * @return Clients count.
 */
uint8_t wifimgr_count_clients();
/**
 * @brief Check station current IP address.
 * @param[out] item - place where received IP address will be written.
 * @return See RETURN_CODES.
 */
RET_CODE wifimgr_get_ip_address(IPAddress* item);
/**
 * @brief Get network name to which module is connected.
 * @param[out] buf - the place, where name of the network will be written.
 * @param[in] buf_size - size of the provided buffer.
 * @return See RETURN_CODES.
 */
RET_CODE wifimgr_get_network_name(char* buf, uint8_t buf_size);
/**
 * @brief Fills up provided array with details of all connected clients.
 * @details
 * It is important, to make sure that provided buffer has enough space.
 * @param[out] buffer - place where details will be written.
 * @return Number of clients written.
 */
uint8_t wifimgr_get_clients_details(ClientID* buffer);
/**
 * @brief Checks how many clients can connect in the same time.
 * @return Maximum clients.
 */
uint8_t wifi_get_max_clients();
/**
 * @brief Register callback function to be called on client event.
 * @param[in] callback - pointer to function
 * @return See RETURN_CODES.
 */
RET_CODE wifimgr_register_data_callback(void(*callback)(ServerClientID id, const char* data));
/**
 * @brief Unregister callback function.
 * @param[in] callback - pointer to function
 * @return See RETURN_CODES.
 */
void wifimgr_unregister_data_callback(void(*callback)(ServerClientID id, const char* data));
/**
 * @brief Resets the WiFi Manager.
 * @details
 * Reset involves also reset of wifi driver.
 * It performs initialization after reset (server is started, NTP time is updated).
 * @return Number of clients written.
 */
RET_CODE wifimgr_reset();
/**
 * @brief Shuts down the WiFi manager.
 * @details
 * It removes all registered callback, closes the running server.
 * @return Number of clients written.
 */
void wifimgr_deinitialize();

