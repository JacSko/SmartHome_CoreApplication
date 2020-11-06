#ifndef _TASK_SCHEDULER_H_
#define _TASK_SCHEDULER_H_

#include "return_codes.h"

enum SchTaskType
{
	SCH_TASK_ONCE,
	SCH_TASK_PERIODIC
};

enum SchTaskState
{
	TASK_RUNNING,
	TASK_STOPPED
};

typedef uint16_t TASK_PERIOD;
typedef void(*TASK) ();

void sch_initialize ();
RET_CODE sch_subscribe (TASK task);
RET_CODE sch_unsubscribe (TASK task);
RET_CODE sch_set_task_period (TASK_PERIOD period);
RET_CODE sch_set_task_state (TASK task, enum SchTaskState state);
RET_CODE sch_set_task_type (TASK task, enum SchTaskType type);
TASK_PERIOD sch_get_task_period (TASK task);
enum SchTaskState sch_get_task_state (TASK task);
enum SchTaskType sch_get_task_type (TASK task);
void sch_task_watcher ();



#endif
