#ifndef _TASK_SCHEDULER_H_
#define _TASK_SCHEDULER_H_
/* ============================= */
/**
 * @file task_scheduler.h
 *
 * @brief Module is responsible for scheduling different task in time.
 * Allows to schedule periodic tasks, that will be trigger until requesting stop.
 * There is possibility to add one time tasks also.
 *
 * @details
 * Module is handling communication with Bluetooth device using UART.
 * Allows to register callback function called on new data received.
 * The string_watcher function have to be called in main thread to
 * receive callback notifications.
 *
 * @author Jacek Skowronek
 * @date 13/12/2020
 */
/* ============================= */

/* =============================
 *  Includes of project headers
 * =============================*/
#include "return_codes.h"
#include "stm32f4xx.h"

/* =============================
 *       Data structures
 * =============================*/
typedef enum SchTaskType
{
	TASKTYPE_TRIGGER,    /**< Need to be manually started */
	TASKTYPE_PERIODIC,   /**< Task will be called in defined period */
	TASKTYPE_ONCE,       /**< Task will be called only once */
	TASKTYPE_UNKNOWN,    /**< Unknown task type - enums count */
}SchTaskType;

typedef enum SchTaskState
{
	TASKSTATE_EMPTY = 0x00, /**< Task is empty and may be replaced by other task */
	TASKSTATE_RUNNING,      /**< Task is running */
	TASKSTATE_STOPPED,      /**< Task stopped and waiting for start */
	TASKSTATE_UNKNOWN,      /**< Unknown task state - enums count */
} SchTaskState;

/* =============================
 *          Defines
 * =============================*/
typedef uint16_t TASK_PERIOD;
typedef void(*TASK) ();


/**
 * @brief Initialize task scheduler.
 * @return None.
 */
void sch_initialize ();
/**
 * @brief Subscribe permanent task to scheduler.
 * @details
 * Permanent means, that even if task became inactive (task type ONCE)
 * it is not removed from internal list ready to become active.
 * Functions set_task_period, set_task_state and set_task_type shall be called afterwards.
 * @param[in] task - Pointer to function
 * @return See RETURN_CODES.
 */
RET_CODE sch_subscribe (TASK task);
/**
 * @brief Unsubscribe permanent task.
 * @param[in] task - Pointer to function
 * @return See RETURN_CODES.
 */
RET_CODE sch_unsubscribe (TASK task);
/**
 * @brief Schedule task to module.
 * @details
 * This task is removed after first call.
 * @param[in] task - Pointer to function
 * @param[in] period - task period
 * @return See RETURN_CODES.
 */
RET_CODE sch_schedule_task (TASK task, TASK_PERIOD period);
/**
 * @brief Set period of task.
 * @details
 * Range: 10ms - 60s
 * @param[in] task - Pointer to task
 * @param[in] period - task period
 * @return See RETURN_CODES.
 */
RET_CODE sch_set_task_period (TASK task, TASK_PERIOD period);
/**
 * @brief Set task state - STOPPED, RUNNING etc.
 * @param[in] task - Pointer to task
 * @param[in] state - task state
 * @return See RETURN_CODES.
 */
RET_CODE sch_set_task_state (TASK task, SchTaskState state);
/**
 * @brief Set task type - PERIODIC, ONCE, etc.
 * @param[in] task - Pointer to task
 * @param[in] type - task type
 * @return See RETURN_CODES.
 */
RET_CODE sch_set_task_type (TASK task, SchTaskType type);
/**
 * @brief Starts task which type is ONCE.
 * @param[in] task - Pointer to task.
 * @return See RETURN_CODES.
 */
RET_CODE sch_trigger_task (TASK task);
/**
 * @brief Get period of the task.
 * @param[in] task - Pointer to task
 * @return Task period.
 */
TASK_PERIOD sch_get_task_period (TASK task);
/**
 * @brief Get state of the task.
 * @param[in] task - Pointer to task
 * @return Task state.
 */
SchTaskState sch_get_task_state (TASK task);
/**
 * @brief Get type of the task.
 * @param[in] task - Pointer to task
 * @return Task type.
 */
SchTaskType sch_get_task_type (TASK task);
/**
 * @brief Watcher - need to be called in main thread loop to call tasks on timeout.
 * @return None.
 */
void sch_task_watcher ();
/**
 * @brief Shuts down task scheduler.
 * @return None.
 */
void sch_deinitialize();



#endif
