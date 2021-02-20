/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "bt_engine.h"
#include "return_codes.h"
#include "simulation_settings.h"
/* =============================
 *          Defines
 * =============================*/
#define BT_ENGINE_CALLBACK_SIZE 5
/* =============================
 *   Internal module functions
 * =============================*/
void btengine_notify_callbacks();
RET_CODE btengine_get_string_from_buffer();
void *bt_thread_execute();
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

typedef struct
{
   uint8_t thread_running = 0;
   pthread_t thread;
   pthread_mutex_t mutex;
} BT_THREAD;
/* =============================
 *      Module variables
 * =============================*/
BT_Config config;
BT_THREAD m_bt_thread;
volatile BT_BUFFER bt_tx_buf;
volatile BT_BUFFER bt_rx_buf;
char* bt_rx_string;
void (*BT_CALLBACKS[BT_ENGINE_CALLBACK_SIZE])(const char *);



RET_CODE btengine_initialize(BT_Config* cfg)
{
   RET_CODE result = RETURN_ERROR;
   config = *cfg;

   bt_tx_buf.buf = (char*) malloc (sizeof(char)*config.buffer_size);
   bt_rx_buf.buf = (char*) malloc (sizeof(char)*config.buffer_size);
   bt_rx_string = (char*) malloc (sizeof(char)*config.string_size);

   bt_tx_buf.head = 0;
   bt_tx_buf.tail = 0;
   bt_tx_buf.string_cnt = 0; /* not used */
   bt_tx_buf.bytes_cnt = 0;  /* not used */

   bt_rx_buf.head = 0;
   bt_rx_buf.tail = 0;
   bt_rx_buf.string_cnt = 0;
   bt_rx_buf.bytes_cnt = 0;

   if (bt_rx_buf.buf && bt_rx_buf.buf && bt_rx_string)
   {

      pthread_mutex_init(&m_bt_thread.mutex, NULL);
      result = RETURN_OK;
   }

   return result;
}

void btengine_deinitialize()
{
   free(bt_tx_buf.buf);
   free(bt_rx_buf.buf);
   free(bt_rx_string);
   bt_tx_buf.buf = NULL;
   bt_rx_buf.buf = NULL;
   bt_rx_string = NULL;
   for (uint8_t i = 0; i < BT_ENGINE_CALLBACK_SIZE; i++)
   {
      BT_CALLBACKS[i] = NULL;
   }
}

void *bt_thread_execute()
{

}

RET_CODE btengine_send_string(const char * buffer)
{
   return btengine_send_bytes((const uint8_t*)buffer, strlen(buffer));
}

RET_CODE btengine_send_bytes(const uint8_t* data, uint16_t size)
{
   if (!bt_tx_buf.buf)
   {
      return RETURN_ERROR;
   }
   //TODO write to socket
}

RET_CODE btengine_can_read_string()
{
   RET_CODE result = RETURN_NOK;

   if (bt_rx_buf.string_cnt > 0)
   {
      btengine_get_string_from_buffer();
      if (strlen(bt_rx_string) > 0)
      {
         result = RETURN_OK;
      }
   }
   return result;
}

const char* btengine_get_string()
{
   return bt_rx_string;
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
   return bt_rx_buf.bytes_cnt;
}

const uint8_t* btengine_get_bytes()
{
   char* buffer = bt_rx_string;
   if (!bt_rx_buf.buf || bt_rx_buf.bytes_cnt == 0 || !buffer)
   {
      return NULL;
   }

   while (bt_rx_buf.bytes_cnt)
   {
      *buffer = bt_rx_buf.buf[bt_rx_buf.tail];
      bt_rx_buf.tail++;
      bt_rx_buf.bytes_cnt--;
      if (bt_rx_buf.tail == config.buffer_size)
      {
         bt_rx_buf.tail = 0;
      }
      buffer++;

   }
   return (uint8_t*)bt_rx_string;
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
      btengine_notify_callbacks();
   }
}

