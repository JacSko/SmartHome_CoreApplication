#ifndef _TIME_COUNTER_MOCK_H_
#define _TIME_COUNTER_MOCK_H_

#include "time_counter.h"
#include "gmock/gmock.h"

struct timestampMock
{
	MOCK_METHOD0(ts_init, void());
	MOCK_METHOD0(ts_deinit, void());
	MOCK_METHOD0(ts_get, uint16_t());
	MOCK_METHOD1(ts_get_diff, uint16_t(uint16_t));
	MOCK_METHOD1(ts_wait, void(uint16_t));
};


timestampMock* ts_mock;

void mock_ts_init()
{
   ts_mock = new (timestampMock);
}

void mock_ts_deinit()
{
	delete ts_mock;
}

uint16_t ts_get()
{
   return ts_mock->ts_get();
}
uint16_t ts_get_diff(uint16_t timestamp)
{
   return ts_mock->ts_get_diff(timestamp);
}
void ts_wait(uint16_t period)
{
   ts_mock->ts_wait(period);
}
#endif
