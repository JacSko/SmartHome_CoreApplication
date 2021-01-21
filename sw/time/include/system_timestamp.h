#ifndef _SYSTEM_TIMESTAMP_H_
#define _SYSTEM_TIMESTAMP_H_

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

/**
 * @brief Initialize module.
 * @return None.
 */
void ts_init();
/**
 * @brief Deinitialize module.
 * @return None.
 */
void ts_deinit();
/**
 * @brief Get current timestamp.
 * @return None.
 */
uint16_t ts_get();
/**
 * @brief Get difference between current and given timestamp.
 * @return None.
 */
uint16_t ts_get_diff(uint16_t timestamp);
/**
 * @brief Suspend program execution for defined time.
 * @return None.
 */
void ts_wait(uint16_t ms);


#endif
