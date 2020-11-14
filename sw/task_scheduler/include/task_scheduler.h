#ifndef _TASK_SCHEDULER_H_
#define _TASK_SCHEDULER_H_

#include "return_codes.h"
#include "stm32f4xx.h"

typedef enum SchTaskType
{
	TASKTYPE_TRIGGER,
	TASKTYPE_PERIODIC,
	TASKTYPE_ONCE,
	TASKTYPE_UNKNOWN,
}SchTaskType;

typedef enum SchTaskState
{
	TASKSTATE_EMPTY = 0x00,
	TASKSTATE_RUNNING,
	TASKSTATE_STOPPED,
	TASKSTATE_UNKNOWN,
} SchTaskState;

typedef uint16_t TASK_PERIOD;
typedef void(*TASK) ();


/*
 * Initialize task scheduler.
 * This means e.g. subscribtion to time counter.
 */
void sch_initialize ();

/*
 * Subscribe permanent task to scheduler.
 * Permanent means, that even if task became inactive (task type ONCE)
 * it is not removed from internal list ready to become active.
 *
 * set_task_period, set_task_state and set_task_type shall be called afterwards.
 * Return:
 * RETURN_OK - when subscription made correctly.
 * RETURN_NOK - when such task is already subscribed.
 * RETURN_ERROR - when e.g max number of subscription made.
 */
RET_CODE sch_subscribe (TASK task);

/*
 * Remove subscription of permanent task from scheduler.
 *
 * Return:
 * RETURN_OK - when subscription removed correctly.
 * RETURN_NOK - when such task was not found.
 */
RET_CODE sch_unsubscribe (TASK task);

/*
 * Add temporary task to scheduler.
 * Task is removed from scheduler just after first call.
 *
 * Return:
 * RETURN_OK - when added correctly.
 * RETURN_NOK - when task not added.
 */
RET_CODE sch_schedule_task (TASK task, TASK_PERIOD period);

/*
 * Set period for task.
 * Period range: 10ms - 60s
 * Return:
 * RETURN_OK - period changed.
 * RETURN_NOK - period cannot be changed (e.g. task not exist or invalid period).
 */
RET_CODE sch_set_task_period (TASK task, TASK_PERIOD period);

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

/*
 * Deinitialize task scheduler.
 * This means e.g. freeing allocated memory.
 */
void sch_deinitialize();



#endif
