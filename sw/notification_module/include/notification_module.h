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


#endif
