#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "return_codes.h"
#include "stm32f4xx.h"


const uint8_t LOGGER_GROUP_ENABLE = 0x01;
const uint8_t LOGGER_GROUP_DISABLE = 0x00;

typedef enum
{
	LOG_DEBUG = 0x00,
	LOG_WIFI_DRIVER = 0x01,
	LOG_TIME = 0x02,
	LOG_TASK_SCHEDULER = 0x03,
	LOG_ERROR = 0x04,

	ENUM_MAX
} LogGroup;

typedef struct
{
	uint16_t buffer_size;
	uint8_t* groups_state;
} LoggerConfig;

RET_CODE logger_initialize(LoggerConfig* config);
RET_CODE logger_enable();
RET_CODE logger_disable();
RET_CODE logger_set_group_state(LogGroup group, uint8_t state);
uint8_t logger_get_group_state(LogGroup group);
void logger_send_log(LogGroup group, const char* data);
void logger_send_if(uint8_t condition, LogGroup group, const char* data);

#endif
