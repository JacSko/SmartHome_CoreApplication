/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdlib.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "task_scheduler.h"
#include "time_counter.h"
#include "Logger.h"
/* =============================
 *          Defines
 * =============================*/
const uint8_t DEFAULT_TASKLIST_SIZE = 20;
const uint8_t DEFAULT_TASK_INCR = 5;
/* =============================
 *       Internal types
 * =============================*/
typedef struct SchItem
{
	TASK callback;
	SchTaskState state;
	SchTaskType type;
	SchTaskPriority priority;
	TASK_PERIOD period;
	TASK_PERIOD count;
}SchItem;

typedef struct SchList
{
	SchItem* list;
	uint8_t size;
	uint8_t capacity;
}SchList;

/* =============================
 *   Internal module functions
 * =============================*/
void sch_on_time_change(TimeItem* item);
void sch_realloc_data(SchItem* source, SchItem* dest);
SchItem* sch_get_item(TASK task);
RET_CODE sch_is_period_correct(TASK_PERIOD period);
void sch_call_tasks (SchTaskPriority prio);
/* =============================
 *      Module variables
 * =============================*/
SchList items_list;
volatile uint8_t sch_time_changed;



void sch_initialize ()
{
	items_list.size = 0;
	items_list.list = (SchItem*) calloc(DEFAULT_TASKLIST_SIZE, sizeof(SchItem));
	items_list.capacity = DEFAULT_TASKLIST_SIZE;
	if (items_list.list)
	{
		time_register_callback(&sch_on_time_change, TIME_PRIORITY_HIGH);
		items_list.capacity = DEFAULT_TASKLIST_SIZE;
	}
	else
	{
		logger_send(LOG_ERROR, __func__, "Cannot allocate buffer!");
	}
}
RET_CODE sch_subscribe (TASK task)
{
	RET_CODE result = RETURN_OK;
	if (task)
	{
		if (items_list.size == items_list.capacity)
		{
			SchItem* new_loc = (SchItem*) calloc((items_list.size + DEFAULT_TASK_INCR), sizeof(SchItem));
			if (!new_loc)
			{
				return RETURN_NOK;
			}
			sch_realloc_data(items_list.list, new_loc);
			items_list.list = new_loc;
			items_list.capacity = items_list.size + DEFAULT_TASK_INCR;
		}

		for (uint8_t i = 0; i < items_list.capacity; i++)
		{
			if (items_list.list[i].state == TASKSTATE_EMPTY)
			{
				items_list.list[i].callback = task;
				items_list.list[i].count = 0;
				items_list.list[i].period = 0;
				items_list.list[i].priority = TASKPRIO_LOW;
				items_list.list[i].state = TASKSTATE_STOPPED;
				items_list.size++;
				break;
			}
		}
	}
	else
	{
		result = RETURN_NOK;
	}
	return result;
}

RET_CODE sch_subscribe_and_set(TASK task, SchTaskPriority prio, TASK_PERIOD period, SchTaskState state, SchTaskType type)
{
   RET_CODE result = RETURN_NOK;
   do
   {
      if (sch_subscribe(task) != RETURN_OK){break;}
      if (sch_set_task_priority(task, prio) != RETURN_OK){break;}
      if (sch_set_task_period(task, period) != RETURN_OK){break;}
      if (sch_set_task_state(task, state) != RETURN_OK){break;}
      if (sch_set_task_type(task, type) != RETURN_OK){break;}
      result = RETURN_OK;
   }while(0);

   return result;
}
RET_CODE sch_unsubscribe (TASK task)
{
	RET_CODE result = RETURN_NOK;

	SchItem* item = sch_get_item(task);
	if (item)
	{
		item->state = TASKSTATE_EMPTY;
		item->callback = NULL;
		items_list.size--;
		result = RETURN_OK;
	}

	return result;
}
RET_CODE sch_schedule_task (TASK task, TASK_PERIOD period)
{
	RET_CODE result = sch_subscribe(task);
	if (result == RETURN_OK)
	{
		result = sch_set_task_period(task, period);
		if (result == RETURN_OK)
		{
			if (sch_set_task_type(task, TASKTYPE_ONCE) == RETURN_OK)
			{
			   sch_set_task_state(task, TASKSTATE_RUNNING);
			}
		}
		else
		{
			sch_unsubscribe(task);
			result = RETURN_NOK;
			logger_send(LOG_ERROR, __func__, "Cannot set task period!");
		}
	}
	else
	{
		logger_send(LOG_ERROR, __func__, "Cannot schedule task!");
	}
	return result;
}
RET_CODE sch_set_task_period (TASK task, TASK_PERIOD period)
{
	RET_CODE result = RETURN_NOK;

	if (sch_is_period_correct (period) == RETURN_OK)
	{
		SchItem* item = sch_get_item(task);
		if (item)
		{
			item->period = period;
			item->count = 0;
			result = RETURN_OK;
		}
	}

	return result;
}
RET_CODE sch_set_task_state (TASK task, enum SchTaskState state)
{
	RET_CODE result = RETURN_NOK;

	SchItem* item = sch_get_item(task);
	if (item && state != TASKSTATE_UNKNOWN)
	{
		item->state = state;
		result = RETURN_OK;
	}
	return result;
}
RET_CODE sch_set_task_type (TASK task, enum SchTaskType type)
{
	RET_CODE result = RETURN_NOK;

	SchItem* item = sch_get_item(task);
	if (item && type != TASKTYPE_UNKNOWN)
	{
		item->type = type;
		result = RETURN_OK;
	}
	return result;
}
RET_CODE sch_set_task_priority (TASK task, SchTaskPriority prio)
{
   RET_CODE result = RETURN_NOK;

   SchItem* item = sch_get_item(task);
   if (item && prio < TASKPRIO_UNKNOWN)
   {
      item->priority = prio;
      result = RETURN_OK;
   }
   return result;
}
RET_CODE sch_trigger_task (TASK task)
{
	RET_CODE result = RETURN_NOK;

	SchItem* item = sch_get_item(task);
	if (item)
	{
		if(item->type == TASKTYPE_TRIGGER)
		{
		   item->count = 0;
			item->state = TASKSTATE_RUNNING;
			result = RETURN_OK;
		}
	}
	return result;
}
TASK_PERIOD sch_get_task_period (TASK task)
{
	TASK_PERIOD result = 0;

	SchItem* item = sch_get_item(task);
	if (item)
	{
		result = item->period;
	}
	return result;
}
enum SchTaskState sch_get_task_state (TASK task)
{
	SchTaskState result = TASKSTATE_UNKNOWN;

	SchItem* item = sch_get_item(task);
	if (item)
	{
		result = item->state;
	}
	return result;
}
enum SchTaskType sch_get_task_type (TASK task)
{
	SchTaskType result = TASKTYPE_UNKNOWN;

	SchItem* item = sch_get_item(task);
	if (item)
	{
		result = item->type;
	}
	return result;
}
void sch_task_watcher ()
{
	if (sch_time_changed)
	{
		sch_time_changed--;
		sch_call_tasks(TASKPRIO_LOW);
	}
}

void sch_call_tasks (SchTaskPriority prio)
{
   for (uint8_t i = 0; i < items_list.capacity; i++)
   {
      SchItem* item = &items_list.list[i];
      if (item->state == TASKSTATE_RUNNING && item->priority == prio)
      {
         item->count += time_get_basetime();
         if (item->count >= item->period)
         {
            item->count = 0;
            if (item->type == TASKTYPE_TRIGGER)
            {
               item->state = TASKSTATE_STOPPED;
            }
            if (item->callback) item->callback();
            if (item->type == TASKTYPE_ONCE)
            {
               sch_unsubscribe(item->callback);
            }
         }
      }
   }
}

void sch_on_time_change(TimeItem* item)
{
   /* This is called from TIME module interrupt, do not place here so many stuff */
	sch_call_tasks(TASKPRIO_HIGH);
	sch_time_changed++;
}

void sch_realloc_data(SchItem* source, SchItem* dest)
{
	for (uint8_t i = 0; i < items_list.capacity; i++)
	{
		dest[i] = source[i];
	}
	free(source);
}

RET_CODE sch_is_period_correct(TASK_PERIOD period)
{
	RET_CODE result = period>=time_get_basetime()? RETURN_OK : RETURN_NOK;
	return result;
}
SchItem* sch_get_item(TASK task)
{
	SchItem* result = NULL;
	for (uint8_t i = 0; i < items_list.capacity; i++)
	{
		if (items_list.list[i].callback == task)
		{
			result = &items_list.list[i];
			break;
		}
	}
	return result;
}

void sch_deinitialize()
{
	time_unregister_callback(&sch_on_time_change);
	free(items_list.list);
	items_list.size = 0;
	items_list.capacity = 0;
}
