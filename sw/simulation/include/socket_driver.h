#ifndef SOCKET_DRIVER_H
#define SOCKET_DRIVER_H

/* ============================= */
/**
 * @file socket_driver.h
 *
 * @brief Module is responsible for handling socket communication. It is operating as a client.
 *
 * @details
 * After calling sockdrv_create() the unique sock_id is returned, it allows to operate on opened socket.
 * When the server connect, the listener is notified with status SOCK_DRV_CONNECTED.
 * Respectively, in case that server disconnects, there is notification with SOCK_DRV_DISCONNECTED sent.
 * After such disconnection, the socket is still opened and is awaiting for next connection.
 * When new string in received, event SOCK_DRV_NEW_DATA is sent to listener.
 *
 *
 * @author Jacek Skowronek
 * @date 13/12/2020
 */
/* ============================= */

/* =============================
 *  Includes of project headers
 * =============================*/
#include <stdint.h>
#include <stddef.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "return_codes.h"
/* =============================
 *          Defines
 * =============================*/
/* =============================
 *       Data structures
 * =============================*/
typedef enum
{
   SOCK_DRV_CONNECTED,
   SOCK_DRV_DISCONNECTED,
   SOCK_DRV_NEW_DATA
} SOCK_DRV_EV;
typedef int8_t sock_id;
typedef void (*SOCKET_DRV_LISTENER)(SOCK_DRV_EV ev, const char* data);

/**
 * @brief Creates the new socket connection.
 * @param[in] ip_address - if NULL, localhost is used.
 * @param[in] port - connection port number
 * @return sock_id to manipulate the connection.
 */
sock_id sockdrv_create(const char* ip_address, uint16_t port);
/**
 * @brief Close socket connection.
 * @param[in] id - id of the connection to close
 * @return None
 */
void sockdrv_close(sock_id id);
/**
 * @brief Writes data to socket.
 * @param[in] id - id of the connection.
 * @param[in] data - pointer to data
 * @param[in] size - data size
 * @return See RETURN_CODES
 */
RET_CODE sockdrv_write(sock_id, const char* data, size_t size);
/**
 * @brief Add connection event listener.
 * @param[in] id - the connection id
 * @param[in] listener - pointer to function
 * @return None
 */
void sockdrv_add_listener(sock_id id, SOCKET_DRV_LISTENER listener);
/**
 * @brief Remove connection event listener.
 * @param[in] id - the connection id
 * @return None
 */
void sockdrv_remove_listener(sock_id id);



#endif
