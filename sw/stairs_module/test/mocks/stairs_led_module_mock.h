#ifndef _STAIRS_LED_MODULE_MOCK_H_
#define _STAIRS_LED_MODULE_MOCK_H_

#include "stairs_led_module.h"
#include "gmock/gmock.h"

struct slmMock
{
   MOCK_METHOD1(slm_initialize, RET_CODE(const SLM_CONFIG*));
   MOCK_METHOD0(slm_deinitialize, void());
   MOCK_METHOD1(slm_get_config, RET_CODE(SLM_CONFIG*));
   MOCK_METHOD0(slm_start_program_alw_on, RET_CODE());
   MOCK_METHOD0(slm_start_program, RET_CODE());
   MOCK_METHOD0(slm_stop_program, RET_CODE());
   MOCK_METHOD0(slm_get_state, SLM_STATE());
   MOCK_METHOD0(slm_get_current_program_id, SLM_PROGRAM_ID());
   MOCK_METHOD1(slm_add_listener, RET_CODE(SLM_LISTENER));
   MOCK_METHOD1(slm_remove_listener, RET_CODE(SLM_LISTENER));
   MOCK_METHOD1(slm_set_current_program_id, RET_CODE(SLM_PROGRAM_ID));
   MOCK_METHOD2(slm_get_program_by_id, RET_CODE(SLM_PROGRAM_ID, SLM_PROGRAM*));
   MOCK_METHOD2(slm_replace_program, RET_CODE(SLM_PROGRAM_ID, const SLM_PROGRAM*));
};

slmMock* slm_mock;

void mock_slm_init()
{
   slm_mock = new slmMock;
}

void mock_slm_deinit()
{
	delete slm_mock;
}
RET_CODE slm_initialize(const SLM_CONFIG* config)
{
   return slm_mock->slm_initialize(config);
}
void slm_deinitialize()
{
   slm_mock->slm_deinitialize();
}
RET_CODE slm_get_config(SLM_CONFIG* buffer)
{
   return slm_mock->slm_get_config(buffer);
}
RET_CODE slm_start_program_alw_on()
{
   return slm_mock->slm_start_program_alw_on();
}
RET_CODE slm_start_program()
{
   return slm_mock->slm_start_program();
}
RET_CODE slm_stop_program()
{
   return slm_mock->slm_stop_program();
}
SLM_STATE slm_get_state()
{
   return slm_mock->slm_get_state();
}
SLM_PROGRAM_ID slm_get_current_program_id()
{
   return slm_mock->slm_get_current_program_id();
}
RET_CODE slm_set_current_program_id(SLM_PROGRAM_ID id)
{
   return slm_mock->slm_set_current_program_id(id);
}
RET_CODE slm_get_program_by_id(SLM_PROGRAM_ID id, SLM_PROGRAM* buffer)
{
   return slm_mock->slm_get_program_by_id(id, buffer);
}
RET_CODE slm_replace_program(SLM_PROGRAM_ID id, const SLM_PROGRAM* program)
{
   return slm_mock->slm_replace_program(id, program);
}
RET_CODE slm_add_listener(SLM_LISTENER listener)
{
   return slm_mock->slm_add_listener(listener);
}
RET_CODE slm_remove_listener(SLM_LISTENER listener)
{
   return slm_mock->slm_remove_listener(listener);
}


#endif
