#ifndef _TIME_COUNTER_H
#define _TIME_COUNTER_H



/* ============================= */
/**
 * @file time_counter.h
 *
 * @brief Module responsible for time counting.
 *
 * @details
 * Module has basetime of 10ms. It is using SysTick to measure time period.
 * There is possibility to set current time (obtained e.g. from NTP server).
 *
 *
 * @author Jacek Skowronek
 * @date 13/12/2020
 */
/* ============================= */

/* =============================
 *  Includes of project headers
 * =============================*/
#include "stm32f4xx.h"
#include "return_codes.h"
/* =============================
 *       Data structures
 * =============================*/
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

typedef enum TimeCallbackPriority
{
   TIME_PRIORITY_HIGH, /**< Callback with this priority will be called directly from interrupt handler */
   TIME_PRIORITY_LOW,  /**< Callback with this priority will be called from main loop */
} TimeCallbackPriority;


/**
 * @brief Initialize module.
 * @return None.
 */
void time_init();

/**
 * @brief Shut down the module.
 * @return None.
 */
void time_deinit();
/**
 * @brief Set current system time as UTC.
 * @details
 * Time is converted to local time taking into consideration winter time flag.
 * @return See RETURN_CODES.
 */
RET_CODE time_set_utc(TimeItem* item);
/**
 * @brief Set winter time flag.
 * @return None.
 */
void time_set_winter_time(uint8_t state);
/**
 * @brief Returns current time.
 * @return Current time.
 */
TimeItem* time_get();
/**
 * @brief Delay execution for defined period.
 * @param[in] timeout - time to wait in ms.
 * @return None.
 */
void time_wait(uint16_t timeout);
/**
 * @brief Register callback to be called on time change.
 * @param[in] callback - Pointer to function
 * @param[in] prio - priority of the callback
 * @return See RETURN_CODES.
 */
RET_CODE time_register_callback(void(*callback)(TimeItem*), TimeCallbackPriority prio);
/**
 * @brief Unregister callback.
 * @param[in] callback - Pointer to function
 * @return See RETURN_CODES.
 */
RET_CODE time_unregister_callback(void(*callback)(TimeItem*));
/**
 * @brief Watcher to be called in main thread loop - responsible for calling callbacks.
 * @return None.
 */
void time_watcher();
/**
 * @brief Returns current base time - e.g. 10ms.
 * @return Time counter basetime.
 */
uint16_t time_get_basetime();



#endif
