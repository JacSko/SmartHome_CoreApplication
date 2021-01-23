#ifndef _BATHROOM_FAN_MOCK_H_
#define _BATHROOM_FAN_MOCK_H_

#include "bathroom_fan.h"
#include "gmock/gmock.h"

struct fanMock
{
   MOCK_METHOD1(fan_initialize, RET_CODE(const FAN_CONFIG*));
   MOCK_METHOD0(fan_deinitialize, void());
   MOCK_METHOD0(fan_start, RET_CODE());
   MOCK_METHOD0(fan_stop, RET_CODE());
   MOCK_METHOD0(fan_get_state, FAN_STATE());
   MOCK_METHOD1(fan_add_listener, RET_CODE(FAN_LISTENER));
   MOCK_METHOD1(fan_remove_listener, RET_CODE(FAN_LISTENER));
   MOCK_METHOD1(fan_set_max_working_time, RET_CODE(uint16_t));
   MOCK_METHOD1(fan_set_min_working_time, RET_CODE(uint16_t));
   MOCK_METHOD1(fan_set_humidity_threshold, RET_CODE(uint8_t));
   MOCK_METHOD1(fan_set_threshold_hysteresis, RET_CODE(uint8_t));
   MOCK_METHOD1(fan_get_config, RET_CODE(FAN_CONFIG*));
};

fanMock* fan_mock;

void mock_fan_init()
{
   fan_mock = new fanMock;
}

void mock_fan_deinit()
{
	delete fan_mock;
}

RET_CODE fan_initialize(const FAN_CONFIG* cfg)
{
   return fan_mock->fan_initialize(cfg);
}
void fan_deinitialize()
{
   fan_mock->fan_deinitialize();
}
RET_CODE fan_start()
{
   return fan_mock->fan_start();
}
RET_CODE fan_stop()
{
   return fan_mock->fan_stop();
}
FAN_STATE fan_get_state()
{
   return fan_mock->fan_get_state();
}
RET_CODE fan_set_max_working_time(uint16_t time_s)
{
   return fan_mock->fan_set_max_working_time(time_s);
}
RET_CODE fan_set_min_working_time(uint16_t time_s)
{
   return fan_mock->fan_set_min_working_time(time_s);
}
RET_CODE fan_set_humidity_threshold(uint8_t hum_trigger)
{
   return fan_mock->fan_set_humidity_threshold(hum_trigger);
}
RET_CODE fan_set_threshold_hysteresis(uint8_t hum_trigger)
{
   return fan_mock->fan_set_threshold_hysteresis(hum_trigger);
}
RET_CODE fan_get_config(FAN_CONFIG* buffer)
{
   return fan_mock->fan_get_config(buffer);
}
RET_CODE fan_add_listener(FAN_LISTENER listener)
{
   return fan_mock->fan_add_listener(listener);
}
RET_CODE fan_remove_listener(FAN_LISTENER listener)
{
   return fan_mock->fan_remove_listener(listener);
}

#endif
