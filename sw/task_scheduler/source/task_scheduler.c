#include "task_scheduler.h"
#include "time_counter.h"



/* Internal functions */
void sch_on_time_change(TimeItem* item);



void sch_initialize ()
{
	time_register_callback(&sch_on_time_change);
}
RET_CODE sch_subscribe (TASK task)
{

}
RET_CODE sch_unsubscribe (TASK task)
{

}
RET_CODE sch_set_task_period (TASK_PERIOD period)
{

}
RET_CODE sch_set_task_state (TASK task, enum SchTaskState state)
{

}
RET_CODE sch_set_task_type (TASK task, enum SchTaskType type)
{

}
RET_CODE sch_trigger_task (TASK task)
{

}
TASK_PERIOD sch_get_task_period (TASK task)
{

}
enum SchTaskState sch_get_task_state (TASK task)
{

}
enum SchTaskType sch_get_task_type (TASK task)
{

}
void sch_task_watcher ()
{

}
void sch_on_time_change(TimeItem* item)
{

}
