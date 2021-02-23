#include "hw_stub.h"
#include "socket_driver.h"
#include "simulation_settings.h"
#include "Logger.h"

#define I2C_BOARD_DATA_SIZE 2
#define WIFIMGR_MAX_CLIENTS 2

typedef enum
{
   I2C_STATE_SET,       /* Sets current state of I2C board - in raw format, e.g. 0xFFFF */
   I2C_STATE_NTF,       /* Event sent to test framework to notify that new data was written to I2C device */
   DHT_STATE_SET,       /* Sets current state of DHT sensor */
   WIFI_CLIENT_ADD,     /* Simulates adding new client connection on wifi interface */
   WIFI_CLIENT_REMOVE,  /* Simulates removing client connection on wifi interface */
   WIFI_TIME_SET,       /* Sets the current network time */
} HW_STUB_EVENT_ID;
typedef struct
{
   I2C_ADDRESS address;
   uint8_t state [I2C_BOARD_DATA_SIZE];
} I2C_BOARD;

typedef struct
{
   DHT_SENSOR sensors[DHT_ENUM_MAX];
} DHT_SENSORS_STUB;

typedef struct
{
   ClientID clients [WIFIMGR_MAX_CLIENTS];
   IPAddress address;
   TimeItem network_time;
} WIFI_STUB;


typedef struct
{
   sock_id control_id;
   sock_id app_ntf_id;
   I2C_BOARD relays_board;
   I2C_BOARD inputs_board;
   I2C_BOARD led_board;
   WIFI_STUB wifi_board;
   DHT_SENSORS_STUB dht_sensors;
   void(*notify_wifimgr)(ClientEvent ev, ServerClientID id, const char* data);
} HWSTUB;


void hwstub_on_new_command(SOCK_DRV_EV ev, const char* data);
void hwstub_on_new_app_data(SOCK_DRV_EV ev, const char* data);
I2C_BOARD* hwstub_get_board_by_address(I2C_ADDRESS addr);


HWSTUB m_hw_stub;


void hwstub_init()
{
   m_hw_stub.control_id = -1;
   m_hw_stub.app_ntf_id = -1;
  /* TODO: import below settings from global settings header */
   m_hw_stub.inputs_board.address = 0x40;
   m_hw_stub.inputs_board.state[0] = 0xFF;
   m_hw_stub.inputs_board.state[1] = 0xFF;
   m_hw_stub.relays_board.address = 0x48;
   m_hw_stub.relays_board.state[0] = 0xFF;
   m_hw_stub.relays_board.state[1] = 0xFF;
   m_hw_stub.led_board.address = 0x4C;
   m_hw_stub.led_board.state[0] = 0xFF;
   m_hw_stub.led_board.state[1] = 0xFF;
   m_hw_stub.notify_wifimgr = NULL;
   IPAddress default_ip = {{0,0,0,0},{0,0,0,0},{0,0,0,0}};
   m_hw_stub.wifi_board.address = default_ip;
   for (uint8_t i = 0; i < WIFIMGR_MAX_CLIENTS; i++)
   {
      m_hw_stub.wifi_board.clients[i].address = default_ip;
      m_hw_stub.wifi_board.clients[i].id = 0;
   }
   TimeItem default_time = {0,0,0,0,0,0,0};
   m_hw_stub.wifi_board.network_time = default_time;


   m_hw_stub.control_id = sockdrv_create(NULL, HW_STUB_CONTROL_PORT);
   m_hw_stub.app_ntf_id = sockdrv_create(NULL, WIFI_NTF_FORWARDING_PORT);
   if (m_hw_stub.control_id < 0)
   {
      logger_send(LOG_ERROR, __func__, "Cannot create control socket");
   }
   else
   {
      sockdrv_add_listener(m_hw_stub.control_id, &hwstub_on_new_command);
   }
   if (m_hw_stub.app_ntf_id < 0)
   {
      logger_send(LOG_ERROR, __func__, "Cannot create app socket");
   }
   else
   {
      sockdrv_add_listener(m_hw_stub.app_ntf_id, &hwstub_on_new_app_data);
   }

}
void hwstub_deinit()
{
   sockdrv_close(m_hw_stub.control_id);
   sockdrv_close(m_hw_stub.app_ntf_id);
   sockdrv_remove_listener(m_hw_stub.control_id);
   sockdrv_remove_listener(m_hw_stub.app_ntf_id);
   m_hw_stub.control_id = -1;
   m_hw_stub.app_ntf_id = -1;
}

void hwstub_on_new_command(SOCK_DRV_EV ev, const char* data)
{
   printf("got new command: %s\n", data? data : "NULL");
}
void hwstub_on_new_app_data(SOCK_DRV_EV ev, const char* data)
{
   printf("got new app_data: %s\n", data? data : "NULL");
}
I2C_BOARD* hwstub_get_board_by_address(I2C_ADDRESS addr)
{
   I2C_BOARD* result = NULL;
   if (addr == m_hw_stub.inputs_board.address)
   {
      result = &m_hw_stub.inputs_board;
   }
   else if (addr == m_hw_stub.relays_board.address)
   {
      result = &m_hw_stub.relays_board;
   }
   else if (addr == m_hw_stub.led_board.address)
   {
      result = &m_hw_stub.led_board;
   }
   return result;
}
/* I2C stub */
RET_CODE hwstub_i2c_read(I2C_ADDRESS address, uint8_t* data, uint8_t size)
{
   RET_CODE result = RETURN_NOK;
   I2C_BOARD* board = hwstub_get_board_by_address(address);
   if (board)
   {
      for (uint8_t i = 0; i < I2C_BOARD_DATA_SIZE; i++)
      {
         data[i] = board->state[i];
      }
      result = RETURN_OK;
   }
   return result;
}
RET_CODE hwstub_i2c_write(I2C_ADDRESS address, const uint8_t* data, uint8_t size)
{
   RET_CODE result = RETURN_NOK;
   I2C_BOARD* board = hwstub_get_board_by_address(address);
   if (board)
   {
      for (uint8_t i = 0; i < I2C_BOARD_DATA_SIZE; i++)
      {
         board->state[i] = data[i];
      }
      result = RETURN_OK;
      //TODO: send notification to test framework
   }
   return result;
}
/* DHT stub */
DHT_STATUS hwstub_dht_read(DHT_SENSOR_ID id, DHT_SENSOR* sensor)
{
   DHT_STATUS result = DHT_STATUS_ERROR;
   if (sensor && id <= DHT_SENSOR6)
   {
      *sensor = m_hw_stub.dht_sensors.sensors[id];
      result = DHT_STATUS_OK;
   }
   return result;
}
/* WIFI stub */
RET_CODE hwstub_wifi_send_data(ServerClientID id, const char* data, uint16_t size)
{
   return sockdrv_write(m_hw_stub.app_ntf_id, data, size);
}
RET_CODE hwstub_wifi_send_bytes(ServerClientID id, const uint8_t* data, uint16_t size)
{
   return hwstub_wifi_send_data(m_hw_stub.app_ntf_id, (const char*)data, size);
}
void hwstub_wifi_set_ip_address(IPAddress* ip_address)
{
   m_hw_stub.wifi_board.address = *ip_address;
}
void hwstub_wifi_get_ip_address(IPAddress* ip_address)
{
   *ip_address = m_hw_stub.wifi_board.address;
}
void hwstub_wifi_get_time(const char* ntp_server, TimeItem* item)
{
   *item = m_hw_stub.wifi_board.network_time;
}
void hwstub_wifi_get_current_network_name(char* buffer, uint8_t size)
{
   strncpy(buffer, "STUBBED_NAME", size);
}
void hwstub_wifi_request_client_details(ClientID* client)
{
   *client = m_hw_stub.wifi_board.clients[client->id];
}
void hwstub_wifi_register_device_event_listener(void(*listener)(ClientEvent ev, ServerClientID id, const char* data))
{
   m_hw_stub.notify_wifimgr = listener;
}

