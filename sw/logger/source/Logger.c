/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
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
typedef struct LOG_GROUP
{
   uint8_t state;
   LogGroup id;
   const char* name;
} LOG_GROUP;
/* =============================
 *      Module variables
 * =============================*/
Logger logger;
RET_CODE (*LOGGER_SENDERS[LOGGER_MAX_SENDERS])(const char *);
LOG_GROUP LOGGER_GROUPS[LOG_ENUM_MAX] = {
      {LOGGER_GROUP_ENABLE, LOG_ERROR, "ERROR"},
      {LOGGER_GROUP_ENABLE, LOG_WIFI_DRIVER, "WIFI_DRV"},
      {LOGGER_GROUP_ENABLE, LOG_TIME, "TIME"},
      {LOGGER_GROUP_ENABLE, LOG_TASK_SCHEDULER, "TASK_SCH"},
      {LOGGER_GROUP_ENABLE, LOG_DEBUG, "DEBUG"},
      {LOGGER_GROUP_ENABLE, LOG_WIFI_MANAGER, "WIFI_MGR"},
      {LOGGER_GROUP_ENABLE, LOG_DHT_DRV, "DHT_DRV"},
      {LOGGER_GROUP_ENABLE, LOG_I2C_DRV, "I2C_DRV"},
      {LOGGER_GROUP_ENABLE, LOG_INPUTS, "INP"},
      {LOGGER_GROUP_ENABLE, LOG_RELAYS, "REL"},
      {LOGGER_GROUP_ENABLE, LOG_FAN, "FAN"},
      {LOGGER_GROUP_ENABLE, LOG_SLM, "SLM"},
      {LOGGER_GROUP_ENABLE, LOG_ENV, "ENV"}};

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
	   LOGGER_GROUPS[i].state = (i == LOG_ERROR)? LOGGER_GROUP_ENABLE : LOGGER_GROUP_DISABLE;
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
		if (LOGGER_GROUPS[group].state != state)
		{
			LOGGER_GROUPS[group].state = state;
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
		result = LOGGER_GROUPS[group].state;
	}
	return result;
}
void logger_send(LogGroup group, const char* prefix, const char* fmt, ...)
{
	if (logger.is_enabled && group < LOG_ENUM_MAX)
	{
		if (LOGGER_GROUPS[group].state == LOGGER_GROUP_ENABLE)
		{
			int length = 0;
			va_list va;
			va_start(va, fmt);
			va_end(va);
			{
				TimeItem* time = time_get();
				int offset = string_format(logger.buffer, "[%.2d-%.2d-%d %.2d:%.2d:%.2d:%.3d] - %s - %s:", time->day, time->month, time->year, time->hour, time->minute, time->second, time->msecond,
				      LOGGER_GROUPS[group].name, prefix);
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
		if (LOGGER_GROUPS[group].state == LOGGER_GROUP_ENABLE)
		{
			int length = 0;
			va_list va;
			va_start(va, fmt);
			va_end(va);
			{
				TimeItem* time = time_get();
				int offset = string_format(logger.buffer, "[%.2d-%.2d-%d %.2d:%.2d:%.2d:%.3d] - %s - %s:", time->day, time->month, time->year, time->hour, time->minute, time->second, time->msecond,
				      LOGGER_GROUPS[group].name, prefix);
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

const char* logger_group_to_string(LogGroup group)
{
   const char* result = "";
   if (group < LOG_ENUM_MAX)
   {
      result = LOGGER_GROUPS[group].name;
   }
   return result;
}
LogGroup logger_string_to_group(const char* name)
{
   LogGroup result = LOG_ENUM_MAX;
   if (name)
   {
      for (uint8_t i = 0; i < LOG_ENUM_MAX; i++)
      {
         if (!strcmp(LOGGER_GROUPS[i].name, name))
         {
            result = LOGGER_GROUPS[i].id;
         }
      }
   }
   return result;
}
