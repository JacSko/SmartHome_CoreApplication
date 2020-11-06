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
	uint8_t msecond;
}TimeItem;


/*
 * Initialize time module, e.g. SysTick configuration
*/
void time_init();

/*
 * Set current system time - to be used e.g. to synchornize time with NTP server
*/
RET_CODE time_set(TimeItem* item);

/*
 * Get current time
*/
RET_CODE time_get(TimeItem* item);

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



#endif
