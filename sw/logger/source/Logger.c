/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdlib.h>
#include <stdarg.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "Logger.h"
#include "time_counter.h"
#include "string_formatter.h"
/* =============================
 *          Defines
 * =============================*/
#define LOGGER_MAX_SENDERS 2
/* =============================
 *   Internal module functions
 * =============================*/
const char* logger_get_group_name(LogGroup group);
void logger_notify_data(const char* data);
/* =============================
 *       Internal types
 * =============================*/
typedef struct
{
	uint8_t is_enabled;
	uint8_t is_initialized;
	char* buffer;
	uint16_t buffer_size;
} Logger;
/* =============================
 *      Module variables
 * =============================*/
Logger logger;
RET_CODE (*LOGGER_SENDERS[LOGGER_MAX_SENDERS])(const char *);
uint8_t LOGGER_GROUPS_STATE [LOG_ENUM_MAX] = {};


RET_CODE logger_initialize(uint16_t buffer_size)
{
	RET_CODE result = RETURN_NOK;

	logger.buffer = (char*) malloc (sizeof(char) * buffer_size);
	if (logger.buffer)
	{
		logger.buffer_size = buffer_size;
		logger.is_initialized = 1;
		result = RETURN_OK;
	}
	for (uint8_t i = 0; i < LOG_ENUM_MAX; i++)
	{
		/* LOG_ERROR group always is ON */
		LOGGER_GROUPS_STATE[i] = i == LOG_ERROR? 1 : 0;
	}
	return result;
}

RET_CODE logger_register_sender(RET_CODE(*send_fnc)(const char*))
{
	RET_CODE result = RETURN_NOK;
	for (uint8_t i=0; i < LOGGER_MAX_SENDERS; i++)
	{
		if (LOGGER_SENDERS[i] == NULL)
		{
			LOGGER_SENDERS[i] = send_fnc;
			result = RETURN_OK;
			break;
		}
	}
	return result;
}

RET_CODE logger_unregister_sender(RET_CODE(*send_fnc)(const char*))
{
	RET_CODE result = RETURN_NOK;
	for (uint8_t i=0; i < LOGGER_MAX_SENDERS; i++)
	{
		if (LOGGER_SENDERS[i] == send_fnc)
		{
			LOGGER_SENDERS[i] = NULL;
			result = RETURN_OK;
		}
	}
	return result;
}

void logger_deinitialize()
{
	free(logger.buffer);
	logger.is_initialized = 0;
	for (uint8_t i = 0; i < LOGGER_MAX_SENDERS; i++)
	{
		LOGGER_SENDERS[i] = NULL;
	}
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
void logger_send(LogGroup group, const char* prefix, const char* fmt, ...)
{
	if (logger.is_enabled && group < LOG_ENUM_MAX)
	{
		if (LOGGER_GROUPS_STATE[group] == 1)
		{
			int length = 0;
			va_list va;
			va_start(va, fmt);
			va_end(va);
			{
				TimeItem* time = time_get();
				int offset = string_format(logger.buffer, "[%.2d-%.2d-%d %.2d:%.2d:%.2d:%.3d] - %s - %s:", time->day, time->month, time->year, time->hour, time->minute, time->second, time->msecond,
																									logger_get_group_name(group), prefix);
				va_start(va, fmt);
				length = sf_format_string(logger.buffer+offset, fmt, va);
				va_end(va);
				logger.buffer[offset + length++] = '\n';
				logger.buffer[offset + length] = 0x00;
				logger_notify_data(logger.buffer);
			}
		}
	}
}

void logger_notify_data(const char* data)
{
	for (uint8_t i = 0; i < LOGGER_MAX_SENDERS; i++)
	{
		if (LOGGER_SENDERS[i] != NULL)
		{
			LOGGER_SENDERS[i](data);
		}
	}
}

void logger_send_if(uint8_t cond_bool, LogGroup group, const char* prefix, const char* fmt, ...)
{
	if (cond_bool != 0 && logger.is_enabled && group < LOG_ENUM_MAX)
	{
		if (LOGGER_GROUPS_STATE[group] == 1)
		{
			int length = 0;
			va_list va;
			va_start(va, fmt);
			va_end(va);
			{
				TimeItem* time = time_get();
				int offset = string_format(logger.buffer, "[%.2d-%.2d-%d %.2d:%.2d:%.2d:%.3d] - %s - %s:", time->day, time->month, time->year, time->hour, time->minute, time->second, time->msecond,
																									logger_get_group_name(group), prefix);
				va_start(va, fmt);
				length = sf_format_string(logger.buffer+offset, fmt, va);
				va_end(va);
				logger.buffer[offset + length++] = '\n';
				logger.buffer[offset + length] = 0x00;
				logger_notify_data(logger.buffer);
			}
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
	case LOG_WIFI_MANAGER:
		return "WIFI_MGR";
	case LOG_DHT_DRV:
	   return "DHT_DRV";
   case LOG_I2C_DRV:
      return "I2C_DRV";
	default:
		return "";
	}

}
