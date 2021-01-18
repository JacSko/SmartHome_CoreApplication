#ifndef _LOGGER_H_
#define _LOGGER_H_
/* ============================= */
/**
 * @file Logger.h
 *
 * @brief Allows to send debugs over UART interface (to both BT and WiFi targets)
 *
 * @details
 * Logger serves basic debug functionality. You have to register at least one sender function,
 * that allows to send logger string.
 * New groups may be added in LogGroup enumeration.
 * New groups have to be also added in logger_get_group_name function.
 * Logger module is disabled by default.
 * All groups are disabled also - except ERROR group).
 *
 * @author Jacek Skowronek
 * @date 13/12/2020
 */
/* ============================= */

/* =============================
 *  Includes of project headers
 * =============================*/
#include <stdint.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "return_codes.h"
#include "string_formatter.h"
/* =============================
 *          Defines
 * =============================*/
#define LOGGER_GROUP_ENABLE  0x01
#define LOGGER_GROUP_DISABLE 0x00
/* =============================
 *       Data structures
 * =============================*/
typedef enum
{
   LOG_ERROR = 0x00,          /**< Sends ERROR messages - this group is enabled by default) */
   LOG_WIFI_DRIVER = 0x01,    /**< Logs from WiFi Driver */
   LOG_TIME = 0x02,           /**< Logs from TIME module */
   LOG_TASK_SCHEDULER = 0x03, /**< Logs from Task Scheduler */
   LOG_DEBUG = 0x04,          /**< Logs from Debug group */
   LOG_WIFI_MANAGER = 0x05,   /**< Logs from WiFi Manager */
   LOG_DHT_DRV,               /**< Logs from DHT driver - received data, etc */
   LOG_I2C_DRV,               /**< Logs from I2C driver - received data, etc */
   LOG_INPUTS,                /**< Logs from Inputs module */
   LOG_RELAYS,                /**< Logs from Relays module */
   LOG_FAN,                   /**< Logs from Bathroom fan module */
   LOG_SLM,                   /**< Logs from StairsLedModule */
   LOG_ENUM_MAX               /**< Enums count */
} LogGroup;

/**
 * @brief Initialize Logger module.
 * @param[in] buffer_size - size of the buffer where logger string is written.
 * @return See RETURN_CODES.
 */
RET_CODE logger_initialize(uint16_t buffer_size);
/**
 * @brief Register sending function to module.
 * @details Function will be called each time when new string should be send.
 * @param[in] send_fnc - pointer to function
 * @return See RETURN_CODES.
 */
RET_CODE logger_register_sender(RET_CODE(*send_fnc)(const char*));
/**
 * @brief Unregister sending function.
 * @param[in] send_fnc - pointer to function
 * @return See RETURN_CODES.
 */
RET_CODE logger_unregister_sender(RET_CODE(*send_fnc)(const char*));
/**
 * @brief Enable logger module.
 * @return See RETURN_CODES.
 */
RET_CODE logger_enable();
/**
 * @brief Disable logger module.
 * @return See RETURN_CODES.
 */
void logger_disable();
/**
 * @brief Enable/Disable defined logger group.
 * @param[in] group - desired group
 * @param[in] state - group state(0-Disable, 1-Enable)
 * @return See RETURN_CODES.
 */
RET_CODE logger_set_group_state(LogGroup group, uint8_t state);
/**
 * @brief Get defined logger group state.
 * @param[in] group - desired group
 * @return Group state(0-Disable, 1-Enable).
 */
uint8_t logger_get_group_state(LogGroup group);
/**
 * @brief Sends log string.
 * @param[in] group - the group to which data is related
 * @param[in] prefix - short prefix added to log string
 * @param[in] fmt - printf-like format of string
 * @param[in] ... - list of arguments to format
 * @return None.
 */
void logger_send(LogGroup group, const char* prefix, const char* fmt, ...);
/**
 * @brief Sends log string conditionally.
 * @param[in] cond_bool - expression (log will be send if this is true.
 * @param[in] group - the group to which data is related
 * @param[in] prefix - short prefix added to log string
 * @param[in] fmt - printf-like format of string
 * @param[in] ... - list of arguments to format
 * @return None.
 */
void logger_send_if(uint8_t cond_bool, LogGroup group, const char* prefix, const char* fmt, ...);
/**
 * @brief Converts log group enum to string.
 * @param[in] group - Group ID.
 * @return String name.
 */
const char* logger_group_to_string(LogGroup group);
/**
 * @brief Converts log group string to group ID.
 * @param[in] group - group name as string.
 * @return Group ID.
 */
LogGroup logger_string_to_group(const char* name);

#endif
