#ifndef _NOTIFICATION_MODULE_H_
#define _NOTIFICATION_MODULE_H_

/* ============================= */
/**
 * @file notification_module.h
 *
 * @brief Module is responsible for sending notifcations to remote device over WiFi
 *
 * @details
 * Notifications means all necessary data for remote device (temperatures, inputs and relays state, etc).
 * Module registers callback function in WiFi driver module
 *
 * @author Jacek Skowronek
 * @date 01/01/2020
 */
/* ============================= */

/* =============================
 *          Defines
 * =============================*/

/* =============================
 *   Includes of common headers
 * =============================*/
#include "stdint.h"
/* =============================
 *  Includes of project headers
 * =============================*/
#include "return_codes.h"
#include "wifi_manager.h"
/* =============================
 *      Global variables
 * =============================*/

/* =============================
 *       Data structures
 * =============================*/

/**
 * @brief Initialize notification manager.
 * @return See RETURN_CODES.
 */
RET_CODE ntfmgr_init();
/**
 * @brief Parse request received.
 * @param[in] id - id of the sending client
 * @param[in] data - request
 * @return See RETURN_CODES.
 */
RET_CODE ntfmgr_parse_request(ServerClientID id, const char* data);
/**
 * @brief Add function which sends the notifications/responses to client. Only one function can be registered!
 * @param[in] id - id of the client
 * @param[in] data - data bytes
 * @param[in] size - size of data bytes
 * @return See RETURN_CODES.
 */
RET_CODE ntfmgr_register_sender(void(*callback)(ServerClientID id, const uint8_t* data, uint16_t size));
/**
 * @brief Remove function which sends the notifications/responses to client.
 * @return See RETURN_CODES.
 */
RET_CODE ntfmgr_unregister_sender();

#endif
