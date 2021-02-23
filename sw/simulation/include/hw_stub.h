#ifndef _HW_STUB_
#define _HW_STUB_

/* ============================= */
/**
 * @file hw_stub.h
 *
 * @brief Set of HW stub functions, which can be called in replaced implementations of drivers (instead of interacting with hardware).
 *
 * @details
 * The input provided to module is received over socket from test framework. The test framework is responsible for setting correct I2C bus buffer and so on.
 *
 * @author Jacek Skowronek
 * @date 23/02/2021
 */
/* ============================= */

#include <stdint.h>

#include "return_codes.h"
#include "i2c_driver.h"
#include "dht_driver.h"
#include "wifi_driver.h"

void hwstub_init();
void hwstub_deinit();

/* I2C stub */
RET_CODE hwstub_i2c_read(I2C_ADDRESS address, uint8_t* data, uint8_t size);
RET_CODE hwstub_i2c_write(I2C_ADDRESS address, const uint8_t* data, uint8_t size);
RET_CODE hwstub_i2c_read_async(I2C_ADDRESS address, uint8_t size, I2C_CALLBACK callback);
RET_CODE hwstub_i2c_write_async(I2C_ADDRESS address, const uint8_t* data, uint8_t size, I2C_CALLBACK callback);
uint16_t hwstub_i2c_get_timeout();
RET_CODE hwstub_i2c_set_timeout(uint16_t timeout);
/* DHT stub */
DHT_STATUS hwstub_dht_read(DHT_SENSOR_ID id, DHT_SENSOR* sensor);
RET_CODE hwstub_dht_read_async(DHT_SENSOR_ID id, DHT_CALLBACK callback);
RET_CODE hwstub_dht_set_timeout(uint16_t timeout);
uint16_t hwstub_dht_get_timeout();
/* WIFI stub */
RET_CODE hwstub_wifi_connect_to_network(const char* ssid, const char* password);
RET_CODE hwstub_wifi_reset();
RET_CODE hwstub_wifi_disconnect_from_network();
RET_CODE hwstub_wifi_set_mac_address(const char* mac);
RET_CODE hwstub_wifi_open_server(uint16_t port);
RET_CODE hwstub_wifi_close_server();
RET_CODE hwstub_wifi_allow_multiple_clients(uint8_t state);
RET_CODE hwstub_wifi_send_data(ServerClientID id, const char* data, uint16_t size);
RET_CODE hwstub_wifi_send_bytes(ServerClientID id, const uint8_t* data, uint16_t size);
RET_CODE hwstub_wifi_set_ip_address(IPAddress* ip_address);
RET_CODE hwstub_wifi_get_ip_address(IPAddress* ip_address);
RET_CODE hwstub_wifi_get_time(const char* ntp_server, TimeItem* item);
RET_CODE hwstub_wifi_get_current_network_name(char* buffer, uint8_t size);
RET_CODE hwstub_wifi_request_client_details(ClientID* client);
RET_CODE hwstub_wifi_register_device_event_listener(void(*listener)(ClientEvent ev, ServerClientID id, const char* data));





#endif
