/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "bt_engine.h"
#include "return_codes.h"
#include "simulation_settings.h"
#include "Logger.h"
#include "socket_driver.h"
/* =============================
 *          Defines
 * =============================*/
#define BT_ENGINE_CALLBACK_SIZE 5
/* =============================
 *   Internal module functions
 * =============================*/
void btengine_notify_callbacks();
RET_CODE btengine_get_string_from_buffer();
void btengine_on_socket_data(SOCK_DRV_EV ev, const char* data);
/* =============================
 *       Internal types
 * =============================*/
typedef struct
{
   char * buf;
   uint16_t tail;
   uint16_t head;
   uint8_t string_cnt;
   uint16_t bytes_cnt;
} BT_BUFFER;
/* =============================
 *      Module variables
 * =============================*/
BT_Config config;
volatile BT_BUFFER bt_rx_buf;
sock_id m_bt_sock_id;
char* bt_rx_string;
void (*BT_CALLBACKS[BT_ENGINE_CALLBACK_SIZE])(const char *);



RET_CODE btengine_initialize(BT_Config* cfg)
{
   RET_CODE result = RETURN_ERROR;
   config = *cfg;

   bt_rx_buf.buf = (char*) malloc (sizeof(char)*config.buffer_size);
   bt_rx_string = (char*) malloc (sizeof(char)*config.string_size);

   bt_rx_buf.head = 0;
   bt_rx_buf.tail = 0;
   bt_rx_buf.string_cnt = 0;
   bt_rx_buf.bytes_cnt = 0;

   if (bt_rx_buf.buf && bt_rx_buf.buf && bt_rx_string)
   {
      m_bt_sock_id = sockdrv_create(NULL, BLUETOOTH_FORWARDING_PORT);
      if (m_bt_sock_id < 0)
      {
         logger_send(LOG_ERROR, __func__, "wrong sock_id");
      }
      else
      {
         sockdrv_add_listener(m_bt_sock_id, &btengine_on_socket_data);
         logger_send(LOG_SIM, __func__, "listener added");
      }
      result = RETURN_OK;
   }

   return result;
}

void btengine_on_socket_data(SOCK_DRV_EV ev, const char* data)
{
   printf("got ev %d, data: %s\n", ev, data? data : "NULL");
   if (ev == SOCK_DRV_NEW_DATA && data)
   {
      uint8_t bytes_handled = 0;
      while(*data)
      {
         bt_rx_buf.buf[bt_rx_buf.head++] = *data;
         if (bt_rx_buf.head == config.buffer_size)
         {
            bt_rx_buf.head = 0;
         }
         data++;
      }
      bt_rx_buf.buf[bt_rx_buf.head++] = '\n';
      bt_rx_buf.string_cnt++;
   }
}

void btengine_deinitialize()
{
   sockdrv_close(m_bt_sock_id);
   sockdrv_remove_listener(m_bt_sock_id);
   free(bt_rx_buf.buf);
   free(bt_rx_string);
   bt_rx_buf.buf = NULL;
   bt_rx_string = NULL;
   for (uint8_t i = 0; i < BT_ENGINE_CALLBACK_SIZE; i++)
   {
      BT_CALLBACKS[i] = NULL;
   }
}

RET_CODE btengine_send_string(const char * buffer)
{
   return btengine_send_bytes((const uint8_t*)buffer, strlen(buffer));
}

RET_CODE btengine_send_bytes(const uint8_t* data, uint16_t size)
{
   return sockdrv_write(m_bt_sock_id, data, size);
}

RET_CODE btengine_can_read_string()
{
   RET_CODE result = RETURN_NOK;

   if (bt_rx_buf.string_cnt > 0)
   {
      result = RETURN_OK;
   }
   return result;
}

const char* btengine_get_string()
{
   btengine_get_string_from_buffer();
   if (strlen(bt_rx_string) > 0)
   {
      return bt_rx_string;
   }
   return NULL;
}

void btengine_clear_rx()
{
   bt_rx_buf.tail = bt_rx_buf.head;
   bt_rx_buf.string_cnt = 0;
   bt_rx_buf.bytes_cnt = 0;
}

RET_CODE btengine_get_string_from_buffer()
{
   char* buffer = bt_rx_string;
   if (!bt_rx_buf.buf || bt_rx_buf.string_cnt == 0 || !buffer)
   {
      return RETURN_ERROR;
   }

   char c;
   while (1)
   {
      c = bt_rx_buf.buf[bt_rx_buf.tail];
      bt_rx_buf.tail++;
      bt_rx_buf.bytes_cnt--;
      if (bt_rx_buf.tail == config.buffer_size)
      {
         bt_rx_buf.tail = 0;
      }
      if (c != '\r' && c != '\n')
      {
         *buffer = c;
      }
      else
      {
         *buffer = 0x00;
      }
      if (c == '\n')
      {
         *buffer = 0x00;
         break;
      }
      buffer++;

   }
   bt_rx_buf.string_cnt--;
   return RETURN_OK;
}

uint16_t btengine_count_bytes()
{
   logger_send(LOG_ERROR, __func__, "not implemented");
   return 0;
}

const uint8_t* btengine_get_bytes()
{
   logger_send(LOG_ERROR, __func__, "not implemented");
   return NULL;
}

RET_CODE btengine_register_callback(void(*callback)(const char *))
{
   RET_CODE result = RETURN_ERROR;
   for (uint8_t i = 0; i < BT_ENGINE_CALLBACK_SIZE; i++)
   {
      if (BT_CALLBACKS[i] == NULL)
      {
         BT_CALLBACKS[i] = callback;
         result = RETURN_OK;
         break;
      }
   }
   return result;
}

RET_CODE btengine_unregister_callback(void(*callback)(const char *))
{
   RET_CODE result = RETURN_ERROR;
   for (uint8_t i = 0; i < BT_ENGINE_CALLBACK_SIZE; i++)
   {
      if (BT_CALLBACKS[i] == callback)
      {
         BT_CALLBACKS[i] = NULL;
         result = RETURN_OK;
         break;
      }
   }
   return result;
}

void btengine_notify_callbacks()
{
   for (uint8_t i = 0; i < BT_ENGINE_CALLBACK_SIZE; i++)
   {
      if (BT_CALLBACKS[i] != NULL)
      {
         BT_CALLBACKS[i](bt_rx_string);
      }
   }
}

void btengine_string_watcher()
{
   if (btengine_can_read_string())
   {
      btengine_get_string();
      btengine_notify_callbacks();
   }
}

