#ifndef _BTENGINE_MOCK_H_
#define _BTENGINE_MOCK_H_

#include "bt_engine.h"
#include "gmock/gmock.h"

struct btEngineMock
{
	MOCK_METHOD1(btengine_initialize, RET_CODE(BT_Config*));
	MOCK_METHOD0(btengine_deinitialize, void());
	MOCK_METHOD1(btengine_send_string, RET_CODE(const char*));
	MOCK_METHOD2(btengine_send_bytes, RET_CODE(const uint8_t*, uint16_t));
	MOCK_METHOD0(btengine_can_read_string, RET_CODE());
	MOCK_METHOD0(btengine_get_string, const char*());
	MOCK_METHOD0(btengine_clear_rx, void());
	MOCK_METHOD0(btengine_count_bytes, uint16_t());
	MOCK_METHOD0(btengine_get_bytes, const uint8_t*());
	MOCK_METHOD0(btengine_string_watcher, void());
	MOCK_METHOD1(btengine_register_callback, RET_CODE(void(*callback)(const char*)));
	MOCK_METHOD1(btengine_unregister_callback, RET_CODE(void(*callback)(const char*)));
};

btEngineMock* btengineMock;

void mock_btengine_init()
{
	btengineMock = new btEngineMock;
}

void mock_btengine_deinit()
{
	delete btengineMock;
}

RET_CODE btengine_initialize(BT_Config* cfg)
{
	return btengineMock->btengine_initialize(cfg);
}

void btengine_deinitialize()
{
	btengineMock->btengine_deinitialize();
}

RET_CODE btengine_send_string(const char * string)
{
	return btengineMock->btengine_send_string(string);
}

RET_CODE btengine_send_bytes(const uint8_t * bytes, uint16_t size)
{
   return btengineMock->btengine_send_bytes(bytes, size);
}

RET_CODE btengine_can_read_string()
{
	return btengineMock->btengine_can_read_string();
}

uint16_t btengine_count_bytes()
{
	return btengineMock->btengine_count_bytes();
}

void btengine_clear_rx()
{
	btengineMock->btengine_clear_rx();
}

const uint8_t* btengine_get_bytes()
{
	return btengineMock->btengine_get_bytes();
}

const char* btengine_get_string()
{
	return btengineMock->btengine_get_string();
}

void btengine_string_watcher()
{
	btengineMock->btengine_string_watcher();
}

RET_CODE btengine_register_callback(void(*callback)(const char *))
{
	return btengineMock->btengine_register_callback(callback);
}

RET_CODE btengine_unregister_callback(void(*callback)(const char *))
{
	return btengineMock->btengine_unregister_callback(callback);
}

#endif
