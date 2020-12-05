#ifndef _TIME_COUNTER_H
#define _TIME_COUNTER_H

#include "stm32f4xx.h"
#include "return_codes.h"


/*
 * This module provides system time.
 * SysTick interrupt is used as time base.
 * Time accuracy is fixed at 10ms to avoid high CPU load.
*/

typedef struct
{
	uint8_t day;
	uint8_t month;
	uint16_t year;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint16_t msecond;
	uint32_t time_raw;
}TimeItem;


/*
 * Initialize time module, e.g. SysTick configuration
*/
void time_init();

/*
 * Deinitialize time module, e.g. removes all callbacks
*/
void time_deinit();
/*
 * Set current system time - to be used e.g. to synchornize time with NTP server
*/
RET_CODE time_set_utc(TimeItem* item);

/*
 * Set current winter time state - needed to convert time from UTC.
*/
void time_set_winter_time(uint8_t state);

/*
 * Get current time
*/
TimeItem* time_get();

/*
 * Wait function. Allow to suspend execution for defined time.
 * Timeout should be multiply of 10ms
*/
void time_wait(uint16_t timeout);

/*
 * Register callback for time notifications
*/
RET_CODE time_register_callback(void(*callback)(TimeItem*));

/*
 * Unregister callback
*/
RET_CODE time_unregister_callback(void(*callback)(TimeItem*));

/*
 * Watcher to be called on main thread - responsible for calling callbacks
*/
void time_watcher();

/*
 * Returns current time base
*/
uint16_t time_get_basetime();



#endif
