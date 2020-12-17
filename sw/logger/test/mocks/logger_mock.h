#ifndef _LOGGER_MOCK_H_
#define _LOGGER_MOCK_H_


#include <stdarg.h>

#include "Logger.h"
#include "string_formatter.h"
#include "gmock/gmock.h"

struct loggerMock
{
	MOCK_METHOD1(logger_initialize, RET_CODE(uint16_t));
	MOCK_METHOD1(logger_register_sender, RET_CODE(RET_CODE(*send_fnc)(const char*)));
	MOCK_METHOD1(logger_unregister_sender, RET_CODE(RET_CODE(*send_fnc)(const char*)));
	MOCK_METHOD0(logger_enable, RET_CODE());
	MOCK_METHOD0(logger_disable, void());
	MOCK_METHOD2(logger_set_group_state, RET_CODE(LogGroup, uint8_t));
	MOCK_METHOD1(logger_get_group_state, uint8_t(LogGroup));
	MOCK_METHOD1(logger_send, void(LogGroup));
	MOCK_METHOD2(logger_send_if, void(uint8_t, LogGroup));
};

::testing::NiceMock<loggerMock>* logger_mock;

void mock_logger_init()
{
	logger_mock = new ::testing::NiceMock<loggerMock>;
}

void mock_logger_deinit()
{
	delete logger_mock;
}

RET_CODE logger_initialize(uint16_t buffer_size)
{
	return logger_mock->logger_initialize(buffer_size);
}

RET_CODE logger_register_sender(RET_CODE(*send_fnc)(const char*))
{
	return logger_mock->logger_register_sender(send_fnc);
}
RET_CODE logger_unregister_sender(RET_CODE(*send_fnc)(const char*))
{
	return logger_mock->logger_unregister_sender(send_fnc);
}
RET_CODE logger_enable()
{
	return logger_mock->logger_enable();
}

void logger_disable()
{
	logger_mock->logger_disable();
}

RET_CODE logger_set_group_state(LogGroup group, uint8_t state)
{
	return logger_mock->logger_set_group_state(group, state);
}

uint8_t logger_get_group_state(LogGroup group)
{
	return logger_mock->logger_get_group_state(group);
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
	default:
		return "";
	}

}

void logger_send(LogGroup group, const char* prefix, const char* fmt, ...)
{
	int length = 0;
	va_list va;
	va_start(va, fmt);
	va_end(va);
	{
		char buf[1024];
		int offset = string_format(buf, "[0-0-0 0:0:0:0] - %s - %s:", logger_get_group_name(group), prefix);
		va_start(va, fmt);
		length = sf_format_string(buf+offset, fmt, va);
		va_end(va);
		buf[offset + length++] = '\n';
		buf[offset + length] = 0x00;
		printf("%s", buf);
	}
	logger_mock->logger_send(group);
}

void logger_send_if(uint8_t cond_bool, LogGroup group, const char* prefix, const char* fmt, ...)
{
	logger_mock->logger_send_if(cond_bool, group);
}


#endif
