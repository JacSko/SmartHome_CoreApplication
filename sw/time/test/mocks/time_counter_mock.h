#ifndef _TIME_COUNTER_MOCK_H_
#define _TIME_COUNTER_MOCK_H_

#include "time_counter.h"
#include "gmock/gmock.h"

struct timeCounerMock
{
	MOCK_METHOD0(time_init, void());
	MOCK_METHOD1(time_set, RET_CODE(TimeItem*));
	MOCK_METHOD0(time_get, TimeItem*());
	MOCK_METHOD1(time_register_callback, RET_CODE(void(*callback)(TimeItem*)));
	MOCK_METHOD1(time_unregister_callback, RET_CODE(void(*callback)(TimeItem*)));
	MOCK_METHOD0(time_watcher, void());
	MOCK_METHOD0(time_get_basetime, uint16_t());
};


timeCounerMock* time_cnt_mock;

void mock_time_counter_init()
{
	time_cnt_mock = new (timeCounerMock);
}

void mock_time_counter_deinit()
{
	delete time_cnt_mock;
}

void time_init()
{
	time_cnt_mock->time_init();
}
RET_CODE time_set(TimeItem* item)
{
	return time_cnt_mock->time_set(item);
}
TimeItem* time_get()
{
	return time_cnt_mock->time_get();
}
RET_CODE time_register_callback(void(*callback)(TimeItem*))
{
	return time_cnt_mock->time_register_callback(callback);
}
RET_CODE time_unregister_callback(void(*callback)(TimeItem*))
{
	return time_cnt_mock->time_unregister_callback(callback);
}

uint16_t time_get_basetime()
{
	return time_cnt_mock->time_get_basetime();
}

void time_watcher()
{
	time_cnt_mock->time_init();
}


#endif
