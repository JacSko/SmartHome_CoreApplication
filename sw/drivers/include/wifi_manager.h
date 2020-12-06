#include "return_codes.h"

#include "wifi_driver.h"
#include "time_counter.h"

//TODO documentation

RET_CODE wifimgr_initialize();

RET_CODE wifimgr_change_network(const char* ssid, const char* pass);

RET_CODE wifimgr_set_ip_address(const char* address);

RET_CODE wifimgr_set_server_port(uint16_t port);

RET_CODE wifimgr_send_data(ServerClientID id, const char* data);

RET_CODE wifimgr_broadcast_data(const char* data);

RET_CODE wifimgr_get_time(TimeItem* item);

uint8_t wifimgr_count_clients();

RET_CODE wifimgr_register_data_callback(void(*callback)(ServerClientID id, const char* data));

void wifimgr_unregister_data_callback(void(*callback)(ServerClientID id, const char* data));

RET_CODE wifimgr_set_max_clients(uint8_t max);

RET_CODE wifimgr_reset();

void wifimgr_deinitialize();

