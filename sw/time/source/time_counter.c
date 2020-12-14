/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdlib.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "time_counter.h"
#include "core_cmFunc.h"
#include "Logger.h"
/* =============================
 *          Defines
 * =============================*/
#define TIME_CNT_CALLBACK_MAX_SIZE 10
#define TIME_BASETIME_MS 10
/* =============================
 *   Internal module functions
 * =============================*/

/* =============================
 *      Module variables
 * =============================*/
TimeItem timestamp;
volatile uint8_t time_changed;
void (*CALLBACKS[TIME_CNT_CALLBACK_MAX_SIZE])(TimeItem*);
uint8_t winter_time_active = 1;
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

void time_deinit()
{
	for (uint8_t i = 0; i < TIME_CNT_CALLBACK_MAX_SIZE; i ++)
	{
		CALLBACKS[i] = NULL;
	}
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

void time_set_winter_time(uint8_t state)
{
	winter_time_active = state;
}

RET_CODE time_set_utc(TimeItem* item)
{
	RET_CODE result = RETURN_NOK;
	if (item)
	{
		if (is_time_ok(item))
		{
			__disable_irq();
			timestamp = *item;
			if (winter_time_active)
			{
				timestamp.hour += 1;
			}
			else
			{
				timestamp.hour += 2;
			}
			if (timestamp.hour > 23)
			{
				timestamp.hour = timestamp.hour % 24;
				timestamp.day++;
				if (timestamp.day > month_day_cnt[timestamp.month])
				{
					timestamp.month++;
					timestamp.day = 1;
					if (timestamp.month > 12)
					{
						timestamp.month = 1;
						timestamp.year++;
					}
				}
			}
			__enable_irq();
			result = RETURN_OK;
		}
	}
	return result;
}

TimeItem* time_get()
{
	return &timestamp;
}

void time_wait(uint16_t timeout)
{
	if (timeout %10 != 0)
	{
		logger_send(LOG_ERROR, __func__, "invalid timeout %d, waiting %d", timeout, timeout - (timeout - timeout%10));
	}

	uint32_t tim = timestamp.time_raw + (timeout - timeout%10);
	while(time_get()->time_raw < tim);

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
	timestamp.time_raw += TIME_BASETIME_MS;
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
	increment_time();
	time_changed++;
}

