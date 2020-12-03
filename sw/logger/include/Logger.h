#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "return_codes.h"
#include "stm32f4xx.h"


#define LOGGER_GROUP_ENABLE  0x01
#define LOGGER_GROUP_DISABLE 0x00

typedef enum
{
	LOG_ERROR = 0x00,
	LOG_WIFI_DRIVER = 0x01,
	LOG_TIME = 0x02,
	LOG_TASK_SCHEDULER = 0x03,
	LOG_DEBUG = 0x04,

	LOG_ENUM_MAX
} LogGroup;


RET_CODE logger_initialize(uint16_t buffer_size, RET_CODE(*send_fnc)(const char*));
RET_CODE logger_enable();
void logger_disable();
RET_CODE logger_set_group_state(LogGroup group, uint8_t state);
uint8_t logger_get_group_state(LogGroup group);
void logger_send_log(LogGroup group, const char* prefix, const char* data);
void logger_send_if(uint8_t cond_bool, LogGroup group, const char* prefix, const char* data);

#endif
