/* =============================
 *   Includes of common headers
 * =============================*/
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "Logger.h"
#include "socket_driver.h"
#include "system_config_values.h"
/* =============================
 *          Defines
 * =============================*/
#define SOCK_DRV_DEF_BUFFER_SIZE 1024
#define SOCK_DRV_MAX_CONNECTIONS 5
/* =============================
 *   Internal module functions
 * =============================*/
void *sockdrv_thread_execute(void* data);
void sockdrv_notify_listeners(SOCK_DRV_EV ev, sock_id id, const char* data);
/* =============================
 *       Internal types
 * =============================*/
typedef struct
{
   char ip_address [64];
   uint16_t port;
} Conn_Prop;
typedef struct
{
   sock_id id;
   int sock_fd;
   uint8_t connected;
   pthread_mutex_t conn_mutex;
   Conn_Prop conn_prop;
   char buffer[SOCK_DRV_DEF_BUFFER_SIZE];
} Conn_Item;

typedef struct
{
   uint8_t conn_count;
   Conn_Item connections[SOCK_DRV_MAX_CONNECTIONS];
   pthread_t thread;
   pthread_mutex_t conn_mutex;
   pthread_cond_t cond_v;
   uint8_t running;
   uint8_t initialized;
} Socket_Driver;

typedef struct
{
   sock_id id;
   SOCKET_DRV_LISTENER listener;
} Socket_Listener;


/* =============================
 *       Global variables
 * =============================*/
Socket_Driver m_sock_drv;
Socket_Listener m_sock_listeners [SOCK_DRV_MAX_CONNECTIONS];

sock_id sockdrv_create(const char* ip_address, uint16_t port)
{
   sock_id result = -1;

   if (m_sock_drv.initialized == 0)
   {
      logger_send(LOG_SIM, __func__, "[%d] initializing, starting thread", port);
      m_sock_drv.initialized = 1;
      m_sock_drv.conn_count = 0;
      pthread_mutex_init(&m_sock_drv.conn_mutex, NULL);
      for (uint8_t i = 0; i < SOCK_DRV_MAX_CONNECTIONS; i++)
      {
         m_sock_listeners[i].id = -1;
         m_sock_listeners[i].listener = NULL;
         m_sock_drv.connections[i].id = -1;
         m_sock_drv.connections[i].sock_fd = -1;
         pthread_mutex_init(&m_sock_drv.connections[i].conn_mutex, NULL);
      }
      pthread_mutex_lock(&m_sock_drv.conn_mutex);
      pthread_create(&m_sock_drv.thread, NULL, sockdrv_thread_execute, &m_sock_drv);
      pthread_cond_wait(&m_sock_drv.cond_v, &m_sock_drv.conn_mutex); /*< wait for thread start */
      pthread_mutex_unlock(&m_sock_drv.conn_mutex);

   }

   pthread_mutex_lock(&m_sock_drv.conn_mutex);
   uint8_t running_connections = m_sock_drv.conn_count;
   pthread_mutex_unlock(&m_sock_drv.conn_mutex);

   if (running_connections < SOCK_DRV_MAX_CONNECTIONS)
   {
      logger_send(LOG_SIM, __func__, "[%d] starting connection: %s:%u", port, ip_address? ip_address : "localhost", port);
      for (uint8_t i = 0; i < SOCK_DRV_MAX_CONNECTIONS; i++)
      {
         if (m_sock_drv.connections[i].id == -1)
         {
            result = i; /* empty place found */
            break;
         }
      }
      m_sock_drv.connections[result].id = result;
      pthread_mutex_lock(&m_sock_drv.connections[result].conn_mutex);
      if (ip_address)
      {
         strcpy(m_sock_drv.connections[result].conn_prop.ip_address, ip_address);
      }
      else
      {
         strcpy(m_sock_drv.connections[result].conn_prop.ip_address, "127.0.0.1");
      }
      m_sock_drv.connections[result].conn_prop.port = port;
      pthread_mutex_unlock(&m_sock_drv.connections[result].conn_mutex);
   }
   else
   {
      logger_send(LOG_SIM, __func__, "[%d] max conn limit reached", port);
   }

   return result;

}
void *sockdrv_thread_execute(void* data)
{
   Socket_Driver* sock_drv = (Socket_Driver*) data;
   pthread_cond_signal(&m_sock_drv.cond_v);
   pthread_mutex_lock(&sock_drv->conn_mutex);
   sock_drv->running = 1;
   uint8_t exit_requested = 0;
   ssize_t bytes_read = 0;
   uint8_t send_notification = 0;
   pthread_mutex_unlock(&sock_drv->conn_mutex);
   uint8_t sock_msg_header [SOCK_MSG_HEADER_SIZE + 1]; /* size of header + NULL char */


   while(!exit_requested)
   {
      for (uint8_t i = 0; i < SOCK_DRV_MAX_CONNECTIONS; i++)
      {
         pthread_mutex_lock(&sock_drv->connections[i].conn_mutex);
         if (sock_drv->connections[i].id != -1 && !sock_drv->connections[i].connected)
         {
            sock_drv->connections[i].sock_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (sock_drv->connections[i].sock_fd >= 0)
            {
               struct timeval tv;
               tv.tv_sec = 0;
               tv.tv_usec = 100000;
               if (setsockopt(sock_drv->connections[i].sock_fd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval)) < 0)
               {
                  logger_send(LOG_ERROR, __func__, "[%d] cannot set socket timeout", sock_drv->connections[i].conn_prop.port);
               }
               struct sockaddr_in servaddr;
               servaddr.sin_family = AF_INET;
               servaddr.sin_addr.s_addr = inet_addr(sock_drv->connections[i].conn_prop.ip_address);
               servaddr.sin_port = htons(sock_drv->connections[i].conn_prop.port);
               if (connect(sock_drv->connections[i].sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0)
               {
                  logger_send(LOG_SIM, __func__, "[%d] connected", sock_drv->connections[i].conn_prop.port);
                  sock_drv->connections[i].connected = 1;
                  send_notification = 1;
               }
               else
               {
                  close(sock_drv->connections[i].sock_fd);
               }
            }
            else
            {
               //error
            }
         }
         pthread_mutex_unlock(&sock_drv->connections[i].conn_mutex);

         if (send_notification == 1)
         {
            sockdrv_notify_listeners(SOCK_DRV_CONNECTED, sock_drv->connections[i].id, "");
            send_notification = 0;
         }
      }
      //try to read out all descriptors
      for (uint8_t i = 0; i < SOCK_DRV_MAX_CONNECTIONS; i++)
      {
         pthread_mutex_lock(&sock_drv->connections[i].conn_mutex);
         if (sock_drv->connections[i].connected)
         {
            /* read header */
            bytes_read = recv(sock_drv->connections[i].sock_fd, sock_msg_header, SOCK_MSG_HEADER_SIZE, 0);
            if (bytes_read == SOCK_MSG_HEADER_SIZE)
            {
               sock_msg_header[SOCK_MSG_HEADER_SIZE] = 0x00;
               uint32_t len_to_read = atoi(sock_msg_header);
               /* read message */
               bytes_read = recv(sock_drv->connections[i].sock_fd, &sock_drv->connections[i].buffer, len_to_read, 0);
               if (bytes_read > 0)
               {
                  sock_drv->connections[i].buffer[bytes_read] = 0x00;
                  send_notification = 1;
               }
            }
            if (bytes_read == 0)
            {
               sockdrv_notify_listeners(SOCK_DRV_DISCONNECTED, sock_drv->connections[i].id, "");
               sock_drv->connections[i].connected = 0;
               close(sock_drv->connections[i].sock_fd);
               sock_drv->connections[i].sock_fd = -1;
               logger_send(LOG_SIM, __func__, "[%d] server disconnected", sock_drv->connections[i].conn_prop.port);
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
               /* no data recevied */
            }
         }
         pthread_mutex_unlock(&sock_drv->connections[i].conn_mutex);
         if (send_notification == 1)
         {
            sockdrv_notify_listeners(SOCK_DRV_NEW_DATA, sock_drv->connections[i].id, sock_drv->connections[i].buffer);
            send_notification = 0;
         }
      }
      pthread_mutex_lock(&sock_drv->conn_mutex);
      exit_requested = !sock_drv->running;
      pthread_mutex_unlock(&sock_drv->conn_mutex);
   }
}
void sockdrv_close(sock_id id)
{
   int8_t conn_remain = -1;
   for (uint8_t i = 0; i < SOCK_DRV_MAX_CONNECTIONS; i++)
   {
      pthread_mutex_lock(&m_sock_drv.connections[i].conn_mutex);
      if (m_sock_drv.connections[i].id == id && m_sock_drv.connections[i].connected)
      {
         m_sock_drv.connections[i].connected = 0;
         m_sock_drv.connections[i].id = -1;
         close(m_sock_drv.connections[i].sock_fd);
         m_sock_drv.connections[i].sock_fd = -1;

         pthread_mutex_lock(&m_sock_drv.conn_mutex);
         conn_remain = --m_sock_drv.conn_count;
         pthread_mutex_unlock(&m_sock_drv.conn_mutex);
      }
      pthread_mutex_unlock(&m_sock_drv.connections[i].conn_mutex);
   }

   if (conn_remain == 0)
   {
      pthread_mutex_lock(&m_sock_drv.conn_mutex);
      m_sock_drv.running = 0;
      m_sock_drv.initialized = 0;
      pthread_mutex_unlock(&m_sock_drv.conn_mutex);
      pthread_join(m_sock_drv.thread, NULL);
   }
}
RET_CODE sockdrv_write(sock_id id, const char* data, size_t size)
{
   RET_CODE result = RETURN_NOK;
   uint8_t mutex_state = -1;
   uint8_t msg_header [SOCK_MSG_HEADER_SIZE + 1];

   for (uint8_t i = 0; i < SOCK_DRV_MAX_CONNECTIONS; i++)
   {
      mutex_state = pthread_mutex_trylock(&m_sock_drv.connections[i].conn_mutex);
      if (m_sock_drv.connections[i].id == id && m_sock_drv.connections[i].connected)
      {
         result = RETURN_OK;
         ssize_t bytes_to_write = size;
         ssize_t bytes_written = 0;
         ssize_t current_write = 0;

         snprintf(msg_header, SOCK_MSG_HEADER_SIZE + 1, "%.4ld", size);
         send(m_sock_drv.connections[i].sock_fd, msg_header, SOCK_MSG_HEADER_SIZE, 0);

         while (bytes_to_write > 0)
         {
            current_write = send(m_sock_drv.connections[i].sock_fd, data + bytes_written, bytes_to_write, 0);
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

      if (mutex_state == 0)
      {
         pthread_mutex_unlock(&m_sock_drv.connections[i].conn_mutex);
      }
   }
   return result;
}
void sockdrv_add_listener(sock_id id, SOCKET_DRV_LISTENER listener)
{
   for (uint8_t i = 0; i < SOCK_DRV_MAX_CONNECTIONS; i++)
   {
      if (m_sock_listeners[i].listener == NULL || m_sock_listeners[i].id == -1)
      {
         m_sock_listeners[i].id = id;
         m_sock_listeners[i].listener = listener;
         break;
      }
   }
}
void sockdrv_remove_listener(sock_id id)
{
   for (uint8_t i = 0; i < SOCK_DRV_MAX_CONNECTIONS; i++)
   {
      if (m_sock_listeners[i].id == id)
      {
         m_sock_listeners[i].id = -1;
         m_sock_listeners[i].listener = NULL;
         break;
      }
   }
}
void sockdrv_notify_listeners(SOCK_DRV_EV ev, sock_id id, const char* data)
{
   for (uint8_t i = 0; i < SOCK_DRV_MAX_CONNECTIONS; i++)
   {
      if (m_sock_listeners[i].id == id && m_sock_listeners[i].listener)
      {
         m_sock_listeners[i].listener(ev, data);
      }
   }
}
