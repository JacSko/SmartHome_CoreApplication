#include <stdlib.h>
#include "time_counter.h"

#define TIME_CNT_CALLBACK_MAX_SIZE 10
#define TIME_BASETIME_MS 10

TimeItem timestamp;
volatile uint8_t time_changed;
void (*CALLBACKS[TIME_CNT_CALLBACK_MAX_SIZE])(TimeItem*);

uint8_t month_day_cnt[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void time_init()
{
	timestamp.day = 1;
	timestamp.month = 1;
	timestamp.year = 2000;
	timestamp.hour = 0;
	timestamp.minute = 0;
	timestamp.second = 0;
	timestamp.msecond = 0;
	/* systick_value = F_CPU/(100/PERIOD[ms]) */
	SysTick_Config(100000000/(1000/TIME_BASETIME_MS));

}

RET_CODE is_time_ok(TimeItem* item)
{
	uint8_t result = 1;
	result &= item->month <= 12;
	if (result == RETURN_OK)
	{
		result &= item->day <= month_day_cnt[item->month];
	}
	result &= item->hour <= 23;
	result &= item->minute <= 59;
	result &= item->second <= 59;
	result &= item->msecond <= 990 && (item->msecond%10) == 0;

	return result? RETURN_OK : RETURN_NOK;

}

RET_CODE time_set(TimeItem* item)
{
	RET_CODE result = RETURN_NOK;
	if (item)
	{
		if (is_time_ok(item))
		{
			timestamp = *item;
			result = RETURN_OK;
		}
	}
	return result;
}

RET_CODE time_get(TimeItem* item)
{
	RET_CODE result = RETURN_NOK;
	if (item)
	{
		*item = timestamp;
		result = RETURN_OK;
	}
	return result;
}

RET_CODE time_register_callback(void(*callback)(TimeItem*))
{
	RET_CODE result = RETURN_NOK;

	for (uint8_t i = 0; i < TIME_CNT_CALLBACK_MAX_SIZE; i++)
	{
		if (CALLBACKS[i] == NULL)
		{
			CALLBACKS[i] = callback;
			result = RETURN_OK;
			break;
		}
	}
	return result;
}

RET_CODE time_unregister_callback(void(*callback)(TimeItem*))
{
	RET_CODE result = RETURN_NOK;
	for (uint8_t i = 0; i < TIME_CNT_CALLBACK_MAX_SIZE; i++)
	{
		if (CALLBACKS[i] == callback)
		{
			CALLBACKS[i] = NULL;
			result = RETURN_OK;
		}
	}
	return result;
}

RET_CODE is_leap_year()
{
	return (timestamp.year % 4) == 0? RETURN_OK : RETURN_NOK;
}

void increment_time()
{
	timestamp.msecond += TIME_BASETIME_MS;
	if (timestamp.msecond >= 1000)
	{
		timestamp.second++;
		timestamp.msecond %= 1000;
	}
	if (timestamp.second == 60)
	{
		timestamp.minute++;
		timestamp.second = 0;
	}
	if (timestamp.minute == 60)
	{
		timestamp.hour++;
		timestamp.minute = 0;
	}
	if (timestamp.hour == 24)
	{
		timestamp.day++;
		timestamp.hour = 0;
		uint8_t exp_days = (is_leap_year() == RETURN_OK) && (timestamp.month == 2)? month_day_cnt[timestamp.month] +1 : month_day_cnt[timestamp.month];
		if (timestamp.day >= exp_days)
		{
			timestamp.month++;
			timestamp.day = 1;
			if (timestamp.month > 12)
			{
				timestamp.year++;
				timestamp.month = 1;
			}
		}
	}


}

void call_callbacks()
{
	for (uint8_t i = 0; i < TIME_CNT_CALLBACK_MAX_SIZE; i++)
	{
		if (CALLBACKS[i] != NULL)
		{
			CALLBACKS[i](&timestamp);
		}
	}
}

void time_watcher()
{
	if (time_changed)
	{
		increment_time();
		time_changed = 0;
		call_callbacks();
	}
}
uint16_t time_get_basetime()
{
	return TIME_BASETIME_MS;
}

void SysTick_Handler(void)
{
	time_changed++;
}

