#include <pthread.h>
#include <stdlib.h>
#include "hw_stub.h"
#include "Logger.h"
#include "inputs_board.h"
#include "system_config_values.h"
#include <string.h>

#define I2C_BOARD_DATA_SIZE 2
#define WIFIMGR_MAX_CLIENTS 2
#define TEST_APP_MAX_MSG_SIZE 1024
#define CLIENT_ID_DEFAULT_VALUE 0xFF
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
   SOCK_DRV_EV ev;
   char buffer [TEST_APP_MAX_MSG_SIZE];
   pthread_mutex_t mutex;
} WIFI_STUB_EVENT;
typedef struct
{
   uint8_t event_received;
   WIFI_STUB_EVENT last_event;
} WIFI_STUB;

typedef struct
{
   char buffer [TEST_APP_MAX_MSG_SIZE];
   uint8_t buffer_changed;
   pthread_mutex_t buffer_mutex;
} HWSTUB_BUFFER;
typedef struct
{
   sock_id control_id;
   sock_id app_ntf_id;
   I2C_BOARD relays_board;
   I2C_BOARD inputs_board;
   I2C_BOARD led_board;
   WIFI_STUB wifi_board;
   DHT_SENSORS_STUB dht_sensors;
   HWSTUB_BUFFER buffer;
   void(*notify_wifimgr)(ClientEvent ev, ServerClientID id, const char* data);
} HWSTUB;


void hwstub_send_i2c_change_notification(I2C_ADDRESS addr, const uint8_t* data, uint16_t size);
void hwstub_parse_command_from_buffer();
uint16_t hwstub_message_to_byte (uint8_t* buffer);
RET_CODE hw_stub_is_message_complete(const uint8_t* buffer, uint16_t size);
I2C_BOARD* hwstub_get_board_by_address(I2C_ADDRESS addr);
RET_CODE hwstub_set_i2c_device_state(I2C_ADDRESS addr, const uint8_t* data);
RET_CODE hwstub_set_dht_device_state(DHT_SENSOR_ID id, const uint8_t* data);
RET_CODE hwstub_handle_wifi_client_event();

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
   m_hw_stub.led_board.state[0] = 0x00;
   m_hw_stub.led_board.state[1] = 0x00;
   m_hw_stub.notify_wifimgr = NULL;
   pthread_mutex_init(&m_hw_stub.buffer.buffer_mutex, NULL);
   pthread_mutex_init(&m_hw_stub.wifi_board.last_event.mutex, NULL);
   m_hw_stub.buffer.buffer_changed = 0;
   m_hw_stub.wifi_board.event_received = 0;

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
   /* this is called from another thread */
   if (ev == SOCK_DRV_NEW_DATA && data)
   {
      pthread_mutex_lock(&m_hw_stub.buffer.buffer_mutex);
      if (strlen(data) < TEST_APP_MAX_MSG_SIZE -1)
      {
         logger_send(LOG_SIM, __func__, "got message %s", data);
         strcpy(m_hw_stub.buffer.buffer, data);
         m_hw_stub.buffer.buffer_changed = 1;
      }
      else
      {
         logger_send(LOG_ERROR, __func__, "message too long: %d", strlen(data));
      }
      pthread_mutex_unlock(&m_hw_stub.buffer.buffer_mutex);
   }
}

void hwstub_watcher()
{
   uint8_t changed = 0;
   uint8_t wifi_event_recv = 0;
   pthread_mutex_lock(&m_hw_stub.buffer.buffer_mutex);
   changed = m_hw_stub.buffer.buffer_changed;
   if (m_hw_stub.buffer.buffer_changed)
   {
      m_hw_stub.buffer.buffer_changed = 0;
   }
   pthread_mutex_unlock(&m_hw_stub.buffer.buffer_mutex);

   pthread_mutex_lock(&m_hw_stub.wifi_board.last_event.mutex);
   wifi_event_recv = m_hw_stub.wifi_board.event_received;
   if (m_hw_stub.wifi_board.event_received)
   {
      m_hw_stub.wifi_board.event_received = 0;
   }
   pthread_mutex_unlock(&m_hw_stub.wifi_board.last_event.mutex);

   if (changed)
   {
      hwstub_parse_command_from_buffer();
   }
   if (wifi_event_recv)
   {
      hwstub_handle_wifi_client_event();
   }
}
void hwstub_parse_command_from_buffer()
{
   uint8_t buffer [256];
   uint8_t bytes_decoded = hwstub_message_to_byte(buffer);
   logger_send(LOG_SIM, __func__, "got %u bytes", bytes_decoded);
   if (hw_stub_is_message_complete(buffer, bytes_decoded) == RETURN_OK)
   {
      switch(buffer[HWSTUB_EVENT_OFFSET])
      {
      case I2C_STATE_SET:
         hwstub_set_i2c_device_state(buffer[HWSTUB_PAYLOAD_START_OFFSET], buffer + HWSTUB_PAYLOAD_START_OFFSET + 1);
         break;
      case DHT_STATE_SET:
         hwstub_set_dht_device_state((DHT_SENSOR_ID)buffer[HWSTUB_PAYLOAD_START_OFFSET], buffer + HWSTUB_PAYLOAD_START_OFFSET + 1);
         break;
      case I2C_INT_TRIGGER:
         inp_on_interrupt_recevied();
         break;
      default:
         logger_send(LOG_ERROR, __func__, "unsupported HWSTUB_EVENT");
         break;
      }
   }
   else
   {
      logger_send(LOG_ERROR, __func__, "message not valid: %s", m_hw_stub.buffer.buffer);
   }
}
uint16_t hwstub_message_to_byte (uint8_t* buffer)
{
   uint16_t str_len = strlen(m_hw_stub.buffer.buffer);

   char local_buf [4] = {}; /* max byte as string is 255 - 3 chars + NULL */
   uint8_t local_buf_idx = 0;
   uint16_t ext_buf_idx = 0;

   for (uint16_t i = 0; i < str_len + 1; i++)
   {
      if (m_hw_stub.buffer.buffer[i] == ' ' || !m_hw_stub.buffer.buffer[i])
      {
         local_buf[local_buf_idx] = 0x00;
         buffer[ext_buf_idx] = atoi(local_buf);
         ext_buf_idx++;
         local_buf_idx = 0;
      }
      else
      {
         local_buf[local_buf_idx] = m_hw_stub.buffer.buffer[i];
         local_buf_idx++;
      }
   }
   return ext_buf_idx;
}
RET_CODE hw_stub_is_message_complete(const uint8_t* buffer, uint16_t size)
{
   RET_CODE result = RETURN_NOK;
   if (buffer[HWSTUB_EVENT_OFFSET] < HW_STUB_EV_ENUM_COUNT &&
       buffer[HWSTUB_PAYLOAD_SIZE_OFFSET] == (size - HWSTUB_HEADER_SIZE))
   {
      result = RETURN_OK;
   }
   return result;
}
RET_CODE hwstub_set_i2c_device_state(I2C_ADDRESS addr, const uint8_t* data)
{
   RET_CODE result = RETURN_NOK;
   I2C_BOARD* board = hwstub_get_board_by_address(addr);
   if (board)
   {
      for (uint8_t i = 0; i < I2C_BOARD_DATA_SIZE; i++)
      {
         board->state[i] = data[i];
      }
      logger_send(LOG_SIM, __func__, "Set I2C device %x: %x %x", addr, data[0], data[1]);
      result = RETURN_OK;
   }
   else
   {
      logger_send(LOG_ERROR, __func__, "I2C device with addr %x not found", addr);
   }
   return result;
}
RET_CODE hwstub_set_dht_device_state(DHT_SENSOR_ID id, const uint8_t* data)
{
   RET_CODE result = RETURN_NOK;
   if (id < DHT_ENUM_MAX)
   {
      m_hw_stub.dht_sensors.sensors[id].type = (DHT_SENSOR_TYPE)data[0];
      m_hw_stub.dht_sensors.sensors[id].id = id;
      m_hw_stub.dht_sensors.sensors[id].data.temp_h = data[1];
      m_hw_stub.dht_sensors.sensors[id].data.temp_l = data[2];
      m_hw_stub.dht_sensors.sensors[id].data.hum_h = data[3];
      m_hw_stub.dht_sensors.sensors[id].data.hum_l = data[4];
      logger_send(LOG_SIM, __func__, "Set DHT device %d: temp %d.%d hum %d.%d", id, data[1], data[2], data[3], data[4]);
      result = RETURN_OK;
   }
   else
   {
      logger_send(LOG_ERROR, __func__, "not supported sensor id: %u", id);
   }
   return result;
}
RET_CODE hwstub_handle_wifi_client_event()
{
   RET_CODE result = RETURN_NOK;
   if (m_hw_stub.notify_wifimgr)
   {
      pthread_mutex_lock(&m_hw_stub.wifi_board.last_event.mutex);
      m_hw_stub.notify_wifimgr(m_hw_stub.wifi_board.last_event.ev, 0, m_hw_stub.wifi_board.last_event.buffer);
      pthread_mutex_unlock(&m_hw_stub.wifi_board.last_event.mutex);
      result = RETURN_OK;
   }
   return result;
}
void hwstub_on_new_app_data(SOCK_DRV_EV ev, const char* data)
{
   /* this is called from another thread */
   logger_send(LOG_SIM, __func__, "got app socket event: %d", ev);
   switch(ev)
   {
   case SOCK_DRV_CONNECTED:
      pthread_mutex_lock(&m_hw_stub.wifi_board.last_event.mutex);
      m_hw_stub.wifi_board.last_event.ev = CLIENT_CONNECTED;
      m_hw_stub.wifi_board.event_received = 1;
      pthread_mutex_unlock(&m_hw_stub.wifi_board.last_event.mutex);
      break;
   case SOCK_DRV_DISCONNECTED:
      pthread_mutex_lock(&m_hw_stub.wifi_board.last_event.mutex);
      m_hw_stub.wifi_board.last_event.ev = CLIENT_DISCONNECTED;
      m_hw_stub.wifi_board.event_received = 1;
      pthread_mutex_unlock(&m_hw_stub.wifi_board.last_event.mutex);
      break;
   case SOCK_DRV_NEW_DATA:
      pthread_mutex_lock(&m_hw_stub.wifi_board.last_event.mutex);
      if (data && (strlen(data) > 0))
      {
         m_hw_stub.wifi_board.last_event.ev = CLIENT_DATA;
         m_hw_stub.wifi_board.event_received = 1;
         strcpy(m_hw_stub.wifi_board.last_event.buffer, data);
      }
      else
      {
         logger_send(LOG_ERROR, __func__, "got empty data");
      }
      pthread_mutex_unlock(&m_hw_stub.wifi_board.last_event.mutex);
      break;
   }

}
void hwstub_send_i2c_change_notification(I2C_ADDRESS addr, const uint8_t* data, uint16_t size)
{
   uint16_t buf_idx = 0;
   pthread_mutex_lock(&m_hw_stub.buffer.buffer_mutex);
   buf_idx += string_format(&m_hw_stub.buffer.buffer[buf_idx], "%.2u %.2u %.2u ", I2C_STATE_NTF, size + 1, addr);
   for (uint16_t i = 0; i < size; i++)
   {
      buf_idx += string_format(&m_hw_stub.buffer.buffer[buf_idx], "%u ", data[i]);
   }

   m_hw_stub.buffer.buffer[buf_idx - 1] = '\n'; //replace last space with delimiter
   m_hw_stub.buffer.buffer[buf_idx] = 0x00;

   if (sockdrv_write(m_hw_stub.control_id, m_hw_stub.buffer.buffer, buf_idx) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "cannot sent data");
   }
   pthread_mutex_unlock(&m_hw_stub.buffer.buffer_mutex);
}
I2C_BOARD* hwstub_get_board_by_address(I2C_ADDRESS addr)
{
   I2C_BOARD* result = NULL;
   if (addr == m_hw_stub.inputs_board.address || addr == m_hw_stub.inputs_board.address + 1)
   {
      result = &m_hw_stub.inputs_board;
   }
   else if (addr == m_hw_stub.relays_board.address || addr == m_hw_stub.relays_board.address + 1)
   {
      result = &m_hw_stub.relays_board;
   }
   else if (addr == m_hw_stub.led_board.address || addr == m_hw_stub.led_board.address + 1)
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
   RET_CODE result = hwstub_set_i2c_device_state(address, data);
   if (result == RETURN_OK)
   {
      hwstub_send_i2c_change_notification(address, data, size);
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
}
void hwstub_wifi_get_ip_address(IPAddress* ip_address)
{
   if (ip_address)
   {
      IPAddress result = {};
      *ip_address = result;
   }
}
void hwstub_wifi_get_time(const char* ntp_server, TimeItem* item)
{
   if (item)
   {
      TimeItem result = {};
      *item = result;
   }
}
void hwstub_wifi_get_current_network_name(char* buffer, uint8_t size)
{
   strncpy(buffer, "STUBBED_NAME", size);
}
void hwstub_wifi_request_client_details(ClientID* client)
{
   if (client)
   {
      ClientID result = {};
      *client = result;
   }
}
void hwstub_wifi_register_device_event_listener(void(*listener)(ClientEvent ev, ServerClientID id, const char* data))
{
   m_hw_stub.notify_wifimgr = listener;
}

