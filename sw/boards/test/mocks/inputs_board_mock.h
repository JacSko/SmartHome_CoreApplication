#ifndef _INPUTS_BOARD_MOCK_H_
#define _INPUTS_BOARD_MOCK_H_

#include "inputs_board.h"
#include "gmock/gmock.h"

struct inpMock
{
   MOCK_METHOD1(inp_initialize, RET_CODE(const INPUTS_CONFIG*));
   MOCK_METHOD0(inp_deinitialize, void());
   MOCK_METHOD1(inp_get, INPUT_STATE(INPUT_ID));
   MOCK_METHOD1(inp_get_all, RET_CODE(INPUT_STATUS*));
   MOCK_METHOD1(inp_read_all, RET_CODE(INPUT_STATUS*));
   MOCK_METHOD1(inp_get_config, RET_CODE(INPUTS_CONFIG*));
   MOCK_METHOD0(inp_enable_interrupt, void());
   MOCK_METHOD0(inp_disable_interrupt, void());
   MOCK_METHOD1(inp_set_debounce_time, RET_CODE(uint16_t));
   MOCK_METHOD0(inp_get_debounce_time, uint16_t());
   MOCK_METHOD1(inp_set_periodic_update_time, RET_CODE(uint16_t));
   MOCK_METHOD0(inp_get_periodic_update_time, uint16_t());
   MOCK_METHOD0(inp_enable_periodic_update, void());
   MOCK_METHOD0(inp_disable_periodic_update, void());
   MOCK_METHOD1(inp_add_input_listener, RET_CODE(INPUT_LISTENER));
   MOCK_METHOD1(inp_remove_input_listener, void(INPUT_LISTENER));
   MOCK_METHOD0(inp_on_interrupt_recevied, void());
};

inpMock* inp_mock;

void mock_inp_init()
{
   inp_mock = new inpMock;
}

void mock_inp_deinit()
{
   delete inp_mock;
}
RET_CODE inp_initialize(const INPUTS_CONFIG* config)
{
   return inp_mock->inp_initialize(config);
}
void inp_deinitialize()
{
   inp_mock->inp_deinitialize();
}
INPUT_STATE inp_get(INPUT_ID id)
{
   return inp_mock->inp_get(id);
}
RET_CODE inp_get_all(INPUT_STATUS* buffer)
{
   return inp_mock->inp_get_all(buffer);
}
RET_CODE inp_read_all(INPUT_STATUS* buffer)
{
   return inp_mock->inp_read_all(buffer);
}
RET_CODE inp_get_config(INPUTS_CONFIG* buffer)
{
   return inp_mock->inp_get_config(buffer);
}
void inp_enable_interrupt()
{
   inp_mock->inp_enable_interrupt();
}
void inp_disable_interrupt()
{
   inp_mock->inp_disable_interrupt();
}
RET_CODE inp_set_debounce_time(uint16_t time)
{
   return inp_mock->inp_set_debounce_time(time);
}
uint16_t inp_get_debounce_time()
{
   return inp_mock->inp_get_debounce_time();
}
RET_CODE inp_set_periodic_update_time(uint16_t period)
{
   return inp_mock->inp_set_periodic_update_time(period);
}
uint16_t inp_get_periodic_update_time()
{
   return inp_mock->inp_get_periodic_update_time();
}
void inp_enable_periodic_update()
{
   inp_mock->inp_enable_periodic_update();
}
void inp_disable_periodic_update()
{
   inp_mock->inp_disable_periodic_update();
}
RET_CODE inp_add_input_listener(INPUT_LISTENER callback)
{
   return inp_mock->inp_add_input_listener(callback);
}
void inp_remove_input_listener(INPUT_LISTENER callback)
{
   inp_mock->inp_remove_input_listener(callback);
}
void inp_on_interrupt_recevied()
{
   inp_mock->inp_on_interrupt_recevied();
}


#endif
