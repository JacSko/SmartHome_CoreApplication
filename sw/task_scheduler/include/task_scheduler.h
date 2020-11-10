#ifndef _TASK_SCHEDULER_H_
#define _TASK_SCHEDULER_H_

#include "return_codes.h"
#include "stm32f4xx.h"

typedef enum SchTaskType
{
	SCH_TASK_ONCE,
	SCH_TASK_PERIODIC
}SchTaskType;

typedef enum SchTaskState
{
	TASK_RUNNING,
	TASK_STOPPED
} SchTaskState;

typedef uint16_t TASK_PERIOD;
typedef void(*TASK) ();


/*
 * Initialize task scheduler.
 * This means e.g. subscribtion to time counter.
 */
void sch_initialize ();

/*
 * Subscribe task to scheduler.
 * set_task_period, set_task_state and set_task_type shall be called afterwards.
 * Return:
 * RETURN_OK - when subscription made correctly.
 * RETURN_NOK - when such task is already subscribed.
 * RETURN_ERROR - when e.g max number of subscription made.
 */
RET_CODE sch_subscribe (TASK task);

/*
 * Remove subscription from scheduler.
 *
 * Return:
 * RETURN_OK - when subscription removed correctly.
 * RETURN_NOK - when such task was not found.
 */
RET_CODE sch_unsubscribe (TASK task);

/*
 * Set period for task.
 * Period range: 10ms - 60s
 * Return:
 * RETURN_OK - period changed.
 * RETURN_NOK - period cannot be changed (e.g. task not exist or invalid period).
 */
RET_CODE sch_set_task_period (TASK_PERIOD period);

/*
 * Set state for task.
 * States: STOPPED, RUNNING
 * Return:
 * RETURN_OK - state changed.
 * RETURN_NOK - state cannot be changed (e.g. task not exist).
 */
RET_CODE sch_set_task_state (TASK task, SchTaskState state);

/*
 * Set type for task.
 * Types: ONCE, PERIODIC.
 * Return:
 * RETURN_OK - type changed.
 * RETURN_NOK - type cannot be changed (e.g. task not exist).
 */
RET_CODE sch_set_task_type (TASK task, SchTaskType type);

/*
 * Starts task which has type == ONCE.
 * Return:
 * RETURN_OK - task started.
 * RETURN_NOK - task not started (e.g. not found or task has type other than ONCE.
 */
RET_CODE sch_trigger_task (TASK task);

/*
 * Get period for task.
 */
TASK_PERIOD sch_get_task_period (TASK task);

/*
 * Get state for task.
 */
SchTaskState sch_get_task_state (TASK task);

/*
 * Get task type.
 */
SchTaskType sch_get_task_type (TASK task);

/*
 * To be called in main function - responsible for scheduling
 */
void sch_task_watcher ();



#endif
