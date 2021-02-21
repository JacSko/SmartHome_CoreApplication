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
   volatile BT_BUFFER* recv_buffer;
   uint16_t recv_buffer_size;
   int sock_fd;
   uint8_t connection_ready;
   uint8_t thread_running;
   pthread_t thread;
   pthread_mutex_t mutex;
} BT_THREAD;
/* =============================
 *      Module variables
 * =============================*/
BT_Config config;
BT_THREAD m_bt_thread;
volatile BT_BUFFER bt_rx_buf;
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
      m_bt_thread.recv_buffer = &bt_rx_buf;
      m_bt_thread.sock_fd = -1;
      m_bt_thread.connection_ready = 0;
      m_bt_thread.recv_buffer_size = config.buffer_size;
      pthread_mutex_init(&m_bt_thread.mutex, NULL);
      pthread_create(&m_bt_thread.thread, NULL, bt_thread_execute, &m_bt_thread);
      result = RETURN_OK;
   }

   return result;
}

void btengine_deinitialize()
{
   pthread_mutex_lock(&m_bt_thread.mutex);
   uint8_t thread_alive = m_bt_thread.thread_running;
   m_bt_thread.thread_running = 0;
   pthread_mutex_unlock(&m_bt_thread.mutex);

   if (thread_alive)
   {
      pthread_join(m_bt_thread.thread, NULL);
   }
   free(bt_rx_buf.buf);
   free(bt_rx_string);
   bt_rx_buf.buf = NULL;
   bt_rx_string = NULL;
   for (uint8_t i = 0; i < BT_ENGINE_CALLBACK_SIZE; i++)
   {
      BT_CALLBACKS[i] = NULL;
   }
}

void *bt_thread_execute(void* data)
{
   BT_THREAD* thread = (BT_THREAD*) data;

   pthread_mutex_lock(&thread->mutex);
   thread->thread_running = 1;
   thread->connection_ready = 0;
   pthread_mutex_unlock(&thread->mutex);

   uint8_t exit_requested = 0;
   char thread_buffer [1024];
   uint16_t thread_buf_idx = 0;

   while(!exit_requested)
   {
      pthread_mutex_lock(&thread->mutex);
      thread->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
      pthread_mutex_unlock(&thread->mutex);
      if (thread->sock_fd < 0)
      {
         logger_send(LOG_ERROR, __func__, "cannot create socket");
         break;
      }
      struct sockaddr_in servaddr;
      servaddr.sin_family = AF_INET;
      servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
      servaddr.sin_port = htons(BLUETOOTH_FORWARDING_PORT);

      if (connect(thread->sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0)
      {
         logger_send(LOG_ERROR, __func__, "connection established, waiting for data");
         pthread_mutex_lock(&thread->mutex);
         thread->connection_ready = 1;
         pthread_mutex_unlock(&thread->mutex);
         ssize_t bytes_read = 1;
         while(!exit_requested) /* keep looping for receiving data */
         {
            bytes_read = recv(thread->sock_fd, &thread_buffer[thread_buf_idx], 1024, MSG_DONTWAIT);
            if (bytes_read > 0)
            {
               thread_buf_idx += bytes_read;
               for (size_t i = thread_buf_idx - bytes_read; i < thread_buf_idx; i++)
               {
                  if (thread_buffer[i] == '\n')
                  {
                     pthread_mutex_lock(&thread->mutex);
                     thread->recv_buffer->string_cnt++;
                     for (size_t i = 0; i < thread_buf_idx; i++)
                     {
                        thread->recv_buffer->buf[thread->recv_buffer->head] = thread_buffer[i];
                        thread->recv_buffer->head++;
                        thread->recv_buffer->bytes_cnt++;
                        if (thread->recv_buffer->head == thread->recv_buffer_size)
                        {
                           thread->recv_buffer->head = 0;
                        }
                     }
                     pthread_mutex_unlock(&thread->mutex);
                     thread_buf_idx = 0;
                     break;
                  }
               }

            }
            else if (bytes_read == 0)
            {
               logger_send(LOG_ERROR, __func__, "server disconnected");
               pthread_mutex_lock(&thread->mutex);
               thread->connection_ready = 0;
               close(thread->sock_fd);
               thread->sock_fd = -1;
               pthread_mutex_unlock(&thread->mutex);
               break;
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
               /* no data recevied, waiting 1ms */
               usleep(1000);
            }

            pthread_mutex_lock(&thread->mutex);
            exit_requested = !m_bt_thread.thread_running;
            pthread_mutex_unlock(&thread->mutex);
         }
      }
      else
      {
         logger_send(LOG_ERROR, __func__, "cannot connect to server");
      }

      pthread_mutex_lock(&thread->mutex);
      exit_requested = !m_bt_thread.thread_running;
      pthread_mutex_unlock(&thread->mutex);

      if (!exit_requested)
      {
         sleep(1);
      }
      else
      {
         pthread_mutex_lock(&thread->mutex);
         thread->connection_ready = 0;
         close(thread->sock_fd);
         thread->sock_fd = -1;
         pthread_mutex_unlock(&thread->mutex);
      }
   }
   pthread_exit(NULL);
}

RET_CODE btengine_send_string(const char * buffer)
{
   return btengine_send_bytes((const uint8_t*)buffer, strlen(buffer));
}

RET_CODE btengine_send_bytes(const uint8_t* data, uint16_t size)
{
   RET_CODE result = RETURN_NOK;
   pthread_mutex_lock(&m_bt_thread.mutex);
   if (m_bt_thread.connection_ready)
   {
      result = RETURN_OK;
      ssize_t bytes_to_write = size;
      ssize_t bytes_written = 0;
      ssize_t current_write = 0;
      while (bytes_to_write > 0)
      {
         current_write = send(m_bt_thread.sock_fd, data + bytes_written, bytes_to_write, 0);
         if (current_write > 0)
         {
            bytes_written += current_write;
         }
         else
         {
            result = RETURN_NOK;
            break;
         }
         bytes_to_write -= bytes_written;
      }
   }
   pthread_mutex_unlock(&m_bt_thread.mutex);

   return result;
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
   pthread_mutex_lock(&m_bt_thread.mutex);
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
   pthread_mutex_unlock(&m_bt_thread.mutex);
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

