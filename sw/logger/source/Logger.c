#include <stdlib.h>

#include "Logger.h"
#include "time_counter.h"
#include "string_formatter.h"

const char* logger_get_group_name(LogGroup group);

uint8_t LOGGER_GROUPS_STATE [LOG_ENUM_MAX] = {};

typedef struct
{
	uint8_t is_enabled;
	uint8_t is_initialized;
	char* buffer;
	uint16_t buffer_size;
} Logger;

Logger logger;
RET_CODE(*send_to_uart)(const char*);

RET_CODE logger_initialize(uint16_t buffer_size, RET_CODE(*send_fnc)(const char*))
{
	RET_CODE result = RETURN_NOK;

	logger.buffer = (char*) malloc (sizeof(char) * buffer_size);
	if (logger.buffer)
	{
		logger.buffer_size = buffer_size;
		logger.is_initialized = 1;
		send_to_uart = send_fnc;
		result = RETURN_OK;
	}
	for (uint8_t i = 0; i < LOG_ENUM_MAX; i++)
	{
		LOGGER_GROUPS_STATE[i] = 0;
	}
	return result;
}

void logger_deinitialize()
{
	free(logger.buffer);
	logger.is_initialized = 0;
	send_to_uart = NULL;
}
RET_CODE logger_enable()
{
	RET_CODE result = RETURN_NOK;

	if (logger.is_initialized)
	{
		logger.is_enabled = 1;
		result = RETURN_OK;
	}

	return result;
}

void logger_disable()
{
	logger.is_enabled = 0;
}
RET_CODE logger_set_group_state(LogGroup group, uint8_t state)
{
	RET_CODE result = RETURN_NOK;

	if (group < LOG_ENUM_MAX)
	{
		if (LOGGER_GROUPS_STATE[group] != state)
		{
			LOGGER_GROUPS_STATE[group] = state;
			result = RETURN_OK;
		}
	}
	return result;
}
uint8_t logger_get_group_state(LogGroup group)
{
	uint8_t result = 255;
	if (group < LOG_ENUM_MAX)
	{
		result = LOGGER_GROUPS_STATE[group];
	}
	return result;
}
void logger_send(LogGroup group, const char* prefix, const char* data)
{
	if (logger.is_enabled && group < LOG_ENUM_MAX)
	{
		if (LOGGER_GROUPS_STATE[group] == 1)
		{
			TimeItem* time = time_get();

			string_format(logger.buffer, "[%d-%d-%d %d:%d:%d:%d] - %s - %s:%s\n", time->day, time->month, time->year, time->hour, time->minute, time->second, time->msecond,
																					logger_get_group_name(group), prefix, data);
			send_to_uart(logger.buffer);
		}
	}
}
void logger_send_if(uint8_t cond_bool, LogGroup group, const char* prefix, const char* data)
{
	if (cond_bool != 0 && logger.is_enabled && group < LOG_ENUM_MAX)
	{
		if (LOGGER_GROUPS_STATE[group] == 1)
		{
			TimeItem* time = time_get();

			string_format(logger.buffer, "[%d-%d-%d %d:%d:%d:%d] - %s - %s:%s\n", time->day, time->month, time->year, time->hour, time->minute, time->second, time->msecond,
																					logger_get_group_name(group), prefix, data);
			send_to_uart(logger.buffer);
		}
	}
}

const char* logger_get_group_name(LogGroup group)
{
	switch(group)
	{
	case LOG_ERROR:
		return "ERROR";
	case LOG_WIFI_DRIVER:
		return "WIFI_DRV";
	case LOG_TIME:
		return "TIME";
	case LOG_TASK_SCHEDULER:
		return "TASK_SCH";
	case LOG_DEBUG:
		return "DEBUG";
	default:
		return "";
	}

}
