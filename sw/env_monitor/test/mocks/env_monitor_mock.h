#ifndef _ENV_MONITOR_MOCK_H_
#define _ENV_MONITOR_MOCK_H_

#include "env_monitor.h"
#include "gmock/gmock.h"

struct envMonitorMock
{
	MOCK_METHOD1(env_initialize, RET_CODE(const ENV_CONFIG*));
	MOCK_METHOD0(env_deinitialize, void());
	MOCK_METHOD2(env_read_sensor, RET_CODE(ENV_ITEM_ID, DHT_SENSOR*));
	MOCK_METHOD2(env_get_sensor_data, RET_CODE(ENV_ITEM_ID, DHT_SENSOR*));
	MOCK_METHOD1(env_get_error_stats, ENV_ERROR_RATE(ENV_ITEM_ID));
	MOCK_METHOD1(env_set_measurement_period, RET_CODE(uint16_t));
	MOCK_METHOD0(env_get_measurement_period, uint16_t());
	MOCK_METHOD2(env_register_listener, RET_CODE(ENV_CALLBACK, ENV_ITEM_ID));
	MOCK_METHOD2(env_unregister_listener, void(ENV_CALLBACK, ENV_ITEM_ID));

};

envMonitorMock* env_mock;

void mock_env_init()
{
   env_mock = new envMonitorMock;
}

void mock_env_deinit()
{
	delete env_mock;
}

RET_CODE env_initialize(const ENV_CONFIG* cfg)
{
   return env_mock->env_initialize(cfg);
}
void env_deinitialize()
{
   env_mock->env_deinitialize();
}
RET_CODE env_read_sensor(ENV_ITEM_ID id, DHT_SENSOR* buffer)
{
   return env_mock->env_read_sensor(id, buffer);
}
RET_CODE env_get_sensor_data(ENV_ITEM_ID id, DHT_SENSOR* buffer)
{
   return env_mock->env_get_sensor_data(id, buffer);
}
ENV_ERROR_RATE env_get_error_stats(ENV_ITEM_ID id)
{
   return env_mock->env_get_error_stats(id);
}
RET_CODE env_set_measurement_period(uint16_t period)
{
   return env_mock->env_set_measurement_period(period);
}
uint16_t env_get_measurement_period()
{
   return env_mock->env_get_measurement_period();
}
RET_CODE env_register_listener(ENV_CALLBACK callback, ENV_ITEM_ID id)
{
   return env_mock->env_register_listener(callback, id);
}
void env_unregister_listener(ENV_CALLBACK callback, ENV_ITEM_ID id)
{
   env_mock->env_unregister_listener(callback, id);
}

#endif
