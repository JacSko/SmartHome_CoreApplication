#ifndef _TASK_SCH_MOCK_H_
#define _TASK_SCH_MOCK_H_

#include "task_scheduler.h"
#include "gmock/gmock.h"

struct taskSchedulerMock
{
	MOCK_METHOD0(sch_initialize, void());
	MOCK_METHOD1(sch_subscribe, RET_CODE(TASK));
	MOCK_METHOD1(sch_unsubscribe, RET_CODE(TASK));
	MOCK_METHOD2(sch_schedule_task, RET_CODE(TASK, TASK_PERIOD));
	MOCK_METHOD2(sch_set_task_period, RET_CODE(TASK, TASK_PERIOD));
	MOCK_METHOD2(sch_set_task_state, RET_CODE(TASK, SchTaskState));
	MOCK_METHOD2(sch_set_task_type, RET_CODE(TASK, SchTaskType));
	MOCK_METHOD1(sch_trigger_task, RET_CODE(TASK));
	MOCK_METHOD1(sch_get_task_period, TASK_PERIOD(TASK));
	MOCK_METHOD1(sch_get_task_state, SchTaskState(TASK));
	MOCK_METHOD1(sch_get_task_type, SchTaskType(TASK));
	MOCK_METHOD0(sch_deinitialize, void());
};

taskSchedulerMock* sch_mock;

void mock_sch_init()
{
	sch_mock = new taskSchedulerMock;
}

void mock_sch_deinit()
{
	delete sch_mock;
}

void sch_initialize ()
{
	sch_mock->sch_initialize();
}
RET_CODE sch_subscribe (TASK task)
{
	return sch_mock->sch_subscribe(task);
}
RET_CODE sch_unsubscribe (TASK task)
{
	return sch_mock->sch_unsubscribe(task);
}
RET_CODE sch_schedule_task (TASK task, TASK_PERIOD period)
{
	return sch_mock->sch_schedule_task(task, period);
}
RET_CODE sch_set_task_period (TASK task, TASK_PERIOD period)
{
	return sch_mock->sch_set_task_period(task, period);
}
RET_CODE sch_set_task_state (TASK task, SchTaskState state)
{
	return sch_mock->sch_set_task_state(task, state);
}
RET_CODE sch_set_task_type (TASK task, SchTaskType type)
{
	return sch_mock->sch_set_task_type(task, type);
}
RET_CODE sch_trigger_task (TASK task)
{
	return sch_mock->sch_trigger_task(task);
}
TASK_PERIOD sch_get_task_period (TASK task)
{
	return sch_mock->sch_get_task_period(task);
}
SchTaskState sch_get_task_state (TASK task)
{
	return sch_mock->sch_get_task_state(task);
}
SchTaskType sch_get_task_type (TASK task)
{
	return sch_mock->sch_get_task_type(task);
}
void sch_task_watcher()
{

}
void sch_deinitialize()
{
	sch_mock->sch_deinitialize();
}

#endif
