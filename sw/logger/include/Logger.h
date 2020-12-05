#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "return_codes.h"
#include "stm32f4xx.h"
#include "string_formatter.h"


#define LOGGER_GROUP_ENABLE  0x01
#define LOGGER_GROUP_DISABLE 0x00

/** Log groups - new logger groups can be add here. */
/** New group should be also added in function "logger_get_group_name" */
typedef enum
{
	LOG_ERROR = 0x00,
	LOG_WIFI_DRIVER = 0x01,
	LOG_TIME = 0x02,
	LOG_TASK_SCHEDULER = 0x03,
	LOG_DEBUG = 0x04,

	LOG_ENUM_MAX
} LogGroup;

/**
 * Initialization of Logger module.
 * Module is disabled by default.
 * All module groups (except LOG_ERROR) are also disabled.
 * buffer_size - buffer size in bytes, to this buffer formatted string are written.
 * send_fnc - pointer to function, which allows to send out formatted data (eg UART).
 * Currently only one send function may be registered.
 */
RET_CODE logger_initialize(uint16_t buffer_size, RET_CODE(*send_fnc)(const char*));

/**
 * Enables logger module.
 * Returns OK if module enabled successfully.
 * Returns NOK in case of errors, like module to initialized before calling enable.
 */
RET_CODE logger_enable();

/**
 * Disables module execution.
 */
void logger_disable();

/**
 * Sets state for defined group.
 * When group is disabled, logs from such groups are not printed.
 * Returns OK if state changed, otherwise NOK.
 */
RET_CODE logger_set_group_state(LogGroup group, uint8_t state);

/**
 * Gets state for defined group.
 * If group not found, 0xFF is returned.
 */
uint8_t logger_get_group_state(LogGroup group);

/**
 * Sends log.
 */
void logger_send(LogGroup group, const char* prefix, const char* fmt, ...);

/**
 * Sends log in cond_bool is true
 */
void logger_send_if(uint8_t cond_bool, LogGroup group, const char* prefix, const char* fmt, ...);

#endif
