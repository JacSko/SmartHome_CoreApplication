#ifndef _RELAYS_BOARD_MOCK_H_
#define _RELAYS_BOARD_MOCK_H_

#include "relays_board.h"
#include "gmock/gmock.h"

struct relMock
{
   MOCK_METHOD1(rel_initialize, RET_CODE(const RELAYS_CONFIG*));
   MOCK_METHOD0(rel_deinitialize, void());
   MOCK_METHOD2(rel_set, RET_CODE(RELAY_ID id, RELAY_STATE state));
   MOCK_METHOD1(rel_get, RELAY_STATE(RELAY_ID));
   MOCK_METHOD1(rel_get_all, RET_CODE(RELAY_STATUS*));
   MOCK_METHOD1(rel_get_config, RET_CODE(RELAYS_CONFIG*));
   MOCK_METHOD1(rel_set_verification_period, RET_CODE(uint16_t));
   MOCK_METHOD0(rel_get_verification_period, uint16_t());
   MOCK_METHOD0(rel_enable_verification, void());
   MOCK_METHOD0(rel_disable_verification, void());
   MOCK_METHOD0(rel_get_verification_state, RET_CODE());
};

relMock* rel_mock;

void mock_rel_init()
{
   rel_mock = new relMock;
}

void mock_rel_deinit()
{
   delete rel_mock;
}
RET_CODE rel_initialize(const RELAYS_CONFIG* config)
{
   return rel_mock->rel_initialize(config);
}
void rel_deinitialize()
{
   rel_mock->rel_deinitialize();
}
RET_CODE rel_set(RELAY_ID id, RELAY_STATE state)
{
   return rel_mock->rel_set(id, state);
}
RELAY_STATE rel_get(RELAY_ID id)
{
   return rel_mock->rel_get(id);
}
RET_CODE rel_get_all(RELAY_STATUS* buffer)
{
   return rel_mock->rel_get_all(buffer);
}
RET_CODE rel_get_config(RELAYS_CONFIG* buffer)
{
   return rel_mock->rel_get_config(buffer);
}
RET_CODE rel_set_verification_period(uint16_t period)
{
   return rel_mock->rel_set_verification_period(period);
}
uint16_t rel_get_verification_period()
{
   return rel_mock->rel_get_verification_period();
}
void rel_enable_verification()
{
   rel_mock->rel_enable_verification();
}
void rel_disable_verification()
{
   rel_mock->rel_disable_verification();
}
RET_CODE rel_get_verification_state()
{
   return rel_mock->rel_get_verification_state();
}

#endif
