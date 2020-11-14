#include "task_scheduler.h"
#include "time_counter.h"
#include <stdlib.h>

typedef struct SchItem
{
	TASK callback;
	SchTaskState state;
	SchTaskType type;
	TASK_PERIOD period;
	TASK_PERIOD count;
}SchItem;

typedef struct SchList
{
	SchItem* list;
	uint8_t size;
	uint8_t capacity;
}SchList;

/* Internal functions */
void sch_on_time_change(TimeItem* item);
void sch_realloc_data(SchItem* source, SchItem* dest);
SchItem* sch_get_item(TASK task);
RET_CODE sch_is_period_correct(TASK_PERIOD period);

/* Module global members */
SchList items_list;
const uint8_t DEFAULT_TASKLIST_SIZE = 20;
const uint8_t DEFAULT_TASK_INCR = 5;
volatile uint8_t time_changed;

void sch_initialize ()
{
	items_list.size = 0;
	items_list.list = (SchItem*) calloc(DEFAULT_TASKLIST_SIZE, sizeof(SchItem));
	items_list.capacity = DEFAULT_TASKLIST_SIZE;
	if (items_list.list)
	{
		time_register_callback(&sch_on_time_change);
		items_list.capacity = DEFAULT_TASKLIST_SIZE;
	}
	else
	{
		//TODO print error
	}
}
RET_CODE sch_subscribe (TASK task)
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
			items_list.list[i].state = TASKSTATE_STOPPED;
			items_list.size++;
			break;
		}
	}

	return RETURN_OK;

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
			sch_set_task_type(task, TASKTYPE_ONCE);
		}
		else
		{
			//cannot set task period
		}
	}
	else
	{
		//cannot schedule task
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
		item->state = TASKSTATE_RUNNING;
		item->type = type;
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
	if (time_changed)
	{
		time_changed--;
		for (uint8_t i = 0; i < items_list.capacity; i++)
		{
			SchItem* item = &items_list.list[i];
			if (item->state == TASKSTATE_RUNNING)
			{
				item->count += time_get_basetime();
				if (item->count >= item->period)
				{
					item->count = 0;
					if (item->callback) item->callback();
					if (item->type == TASKTYPE_TRIGGER)
					{
						item->state = TASKSTATE_STOPPED;
					}
					if (item->type == TASKTYPE_ONCE)
					{
						sch_unsubscribe(item->callback);
					}
				}
			}
		}
	}
}
void sch_on_time_change(TimeItem* item)
{
	time_changed++;
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
