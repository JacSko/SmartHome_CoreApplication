#ifndef _UARTENGINE_MOCK_H_
#define _UARTENGINE_MOCK_H_

#include "uart_engine.h"
#include "gmock/gmock.h"

struct uartMock
{
	MOCK_METHOD1(uartengine_initialize, RET_CODE(UART_Config*));
	MOCK_METHOD0(uartengine_deinitialize, void());
	MOCK_METHOD1(uartengine_send_string, RET_CODE(const char*));
	MOCK_METHOD0(uartengine_can_read_string, RET_CODE());
	MOCK_METHOD0(uartengine_get_string, const char*());
	MOCK_METHOD0(uartengine_count_bytes, uint16_t());
	MOCK_METHOD0(uartengine_get_bytes, const uint8_t*());
	MOCK_METHOD0(uartengine_clear_rx, void());
	MOCK_METHOD0(uartengine_string_watcher, void());
	MOCK_METHOD1(uartengine_register_callback, RET_CODE(void(*callback)(const char*)));
	MOCK_METHOD1(uartengine_unregister_callback, RET_CODE(void(*callback)(const char*)));
};

uartMock* uartengineMock;

void mock_uartengine_init()
{
	uartengineMock = new uartMock;
}

void mock_uartengine_deinit()
{
	delete uartengineMock;
}

RET_CODE uartengine_initialize(UART_Config* cfg)
{
	return uartengineMock->uartengine_initialize(cfg);
}

void uartengine_deinitialize()
{
	uartengineMock->uartengine_deinitialize();
}

RET_CODE uartengine_send_string(const char * string)
{
	return uartengineMock->uartengine_send_string(string);
}

RET_CODE uartengine_can_read_string()
{
	return uartengineMock->uartengine_can_read_string();
}

uint16_t uartengine_count_bytes()
{
	return uartengineMock->uartengine_count_bytes();
}
void uartengine_clear_rx()
{
	uartengineMock->uartengine_clear_rx();
}

const uint8_t* uartengine_get_bytes()
{
	return uartengineMock->uartengine_get_bytes();
}

const char* uartengine_get_string()
{
	return uartengineMock->uartengine_get_string();
}

void uartengine_string_watcher()
{
	uartengineMock->uartengine_string_watcher();
}

RET_CODE uartengine_register_callback(void(*callback)(const char *))
{
	return uartengineMock->uartengine_register_callback(callback);
}

RET_CODE uartengine_unregister_callback(void(*callback)(const char *))
{
	return uartengineMock->uartengine_unregister_callback(callback);
}

#endif
