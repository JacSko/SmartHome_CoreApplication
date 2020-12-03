#ifndef _LOGGER_MOCK_H_
#define _LOGGER_MOCK_H_

#include "Logger.h"
#include "gmock/gmock.h"

struct loggerMock
{
	MOCK_METHOD2(logger_initialize, RET_CODE(uint16_t, RET_CODE(*)(const char*)));
	MOCK_METHOD0(logger_enable, RET_CODE());
	MOCK_METHOD0(logger_disable, void());
	MOCK_METHOD2(logger_set_group_state, RET_CODE(LogGroup, uint8_t));
	MOCK_METHOD1(logger_get_group_state, uint8_t(LogGroup));
	MOCK_METHOD3(logger_send, void(LogGroup, const char*, const char*));
	MOCK_METHOD4(logger_send_if, void(uint8_t, LogGroup, const char*, const char*));
};

loggerMock* logger_mock;

void mock_logger_init()
{
	logger_mock = new loggerMock;
}

void mock_logger_deinit()
{
	delete logger_mock;
}

RET_CODE logger_initialize(uint16_t buffer_size, RET_CODE(*send_fnc)(const char*))
{
	return logger_mock->logger_initialize(buffer_size, send_fnc);
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

void logger_send(LogGroup group, const char* prefix, const char* data)
{
	logger_mock->logger_send(group, prefix, data);
}

void logger_send_if(uint8_t cond_bool, LogGroup group, const char* prefix, const char* data)
{
	logger_mock->logger_send_if(cond_bool, group, prefix, data);
}


#endif
