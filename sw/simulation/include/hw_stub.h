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
 * When the system under test writes the data over e.g I2C bus, the notification to test framework is sent.
 *
 * COMMAND REFERENCES:
 *                     /============ HEADER ============//========================= PAYLOAD =========================/
 * I2C_STATE_SET:      [HW_STUB_EVENT_ID] [PAYLOAD_SIZE] [I2C_ADDRESS] [I2C_BYTE0] [I2C_BYTE N-1]
 * I2C_STATE_NTF:      [HW_STUB_EVENT_ID] [PAYLOAD_SIZE] [I2C_ADDRESS] [I2C_BYTE0] [I2C_BYTE N-1]
 * DHT_STATE_SET:      [HW_STUB_EVENT_ID] [PAYLOAD_SIZE] [SENSOR_ID] [SENSOR_TYPE] [TEMP_H] [TEMP_L] [HUM_H] [HUM_L]
 * WIFI_CLIENT_ADD:    [HW_STUB_EVENT_ID] [PAYLOAD_SIZE] [CLIENT_ID] [IP_ADDRESS_1] [IP_ADDRESS_2] [IP_ADDRESS_3] [IP_ADDRESS_4]
 * WIFI_CLIENT_REMOVE: [HW_STUB_EVENT_ID] [PAYLOAD_SIZE] [CLIENT_ID] [IP_ADDRESS_1] [IP_ADDRESS_2] [IP_ADDRESS_3] [IP_ADDRESS_4]
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
#include "socket_driver.h"

#define HWSTUB_EVENT_OFFSET 0
#define HWSTUB_PAYLOAD_SIZE_OFFSET 1
#define HWSTUB_PAYLOAD_START_OFFSET 2
#define HWSTUB_HEADER_SIZE 2

typedef enum
{
/* Below enumerations are cloned in test framework, any change here must lead to change in both places*/

   I2C_STATE_SET = 1,       /*< Sets current state of I2C board - in raw format, e.g. 0xFFFF */
   I2C_STATE_NTF = 2,       /*< Event sent to test framework to notify that new data was written to I2C device */
   DHT_STATE_SET = 3,       /*< Sets current state of DHT sensor */
   HW_STUB_EV_ENUM_COUNT,
} HW_STUB_EVENT_ID;



void hwstub_init();
void hwstub_deinit();

/* I2C stub */
RET_CODE hwstub_i2c_read(I2C_ADDRESS address, uint8_t* data, uint8_t size);
RET_CODE hwstub_i2c_write(I2C_ADDRESS address, const uint8_t* data, uint8_t size);
/* DHT stub */
DHT_STATUS hwstub_dht_read(DHT_SENSOR_ID id, DHT_SENSOR* sensor);
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
void hwstub_on_new_command(SOCK_DRV_EV ev, const char* data);
void hwstub_on_new_app_data(SOCK_DRV_EV ev, const char* data);
void hwstub_wifi_set_ip_address(IPAddress* ip_address);
void hwstub_wifi_get_ip_address(IPAddress* ip_address);
void hwstub_wifi_get_time(const char* ntp_server, TimeItem* item);
void hwstub_wifi_get_current_network_name(char* buffer, uint8_t size);
void hwstub_wifi_request_client_details(ClientID* client);
void hwstub_wifi_register_device_event_listener(void(*listener)(ClientEvent ev, ServerClientID id, const char* data));
void hwstub_watcher();





#endif
