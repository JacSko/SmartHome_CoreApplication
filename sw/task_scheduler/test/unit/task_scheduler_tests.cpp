#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/task_scheduler.c"
//#include "../../../../ext_lib/CMSIS/stubs/device/stm32f4xx.h"
#ifdef __cplusplus
}
#endif

#include "time_counter_mock.h"

/**
 * @brief Unit test of task scheduler module.
 *
 * All tests that verify behavior of scheduler module
 *
 * @file task_scheduler_tests.cpp
 * @author  Jacek Skowronek
 * @date    11/11/2020
 */


using namespace ::testing;

struct callbackMock
{
	MOCK_METHOD0(task1_callback, void());
	MOCK_METHOD0(task2_callback, void());
	MOCK_METHOD0(task3_callback, void());
};

callbackMock* callMock;

void fake_callback1()
{
	callMock->task1_callback();
}
void fake_callback2()
{
	callMock->task2_callback();
}
void fake_callback3()
{
	callMock->task3_callback();
}

struct timeFixture : public ::testing::Test
{
	virtual void SetUp()
	{
		mock_time_counter_init();
		callMock = new callbackMock();
	}

	virtual void TearDown()
	{
		mock_time_counter_deinit();
		delete callMock;
	}
};

/**
 * @test Module initialization, adding simple tasks
 */
TEST_F(timeFixture, module_init_task_adding)
{
	/**
	 * @<b>scenario<\b>: Module initialisation.
	 * @<b>expected<\b>: Callback registered.
	 */
	EXPECT_CALL(*time_cnt_mock, time_register_callback(_));
	sch_initialize();

	/**
	 * @<b>scenario<\b>: First task added.
	 * @<b>expected<\b>: Task added correctly.
	 */
	EXPECT_CALL(*time_cnt_mock, time_get_basetime()).WillOnce(Return(10));
	EXPECT_EQ(RETURN_OK, sch_subscribe(&fake_callback1));
	EXPECT_EQ(RETURN_OK, sch_set_task_state(&fake_callback1, TASKSTATE_RUNNING));
	EXPECT_EQ(RETURN_OK, sch_set_task_period(&fake_callback1, 1000));
	EXPECT_EQ(items_list.size, 1);

	/**
	 * @<b>scenario<\b>: First task removed.
	 * @<b>expected<\b>: Task removed correctly.
	 */
	EXPECT_EQ(RETURN_OK, sch_unsubscribe(&fake_callback1));
	EXPECT_EQ(items_list.size, 0);

	EXPECT_CALL(*time_cnt_mock, time_unregister_callback(_));
	sch_deinitialize();
}

/**
 * @test Task adding/removing test
 */
TEST_F(timeFixture, task_add_remove_test)
{
	EXPECT_CALL(*time_cnt_mock, time_register_callback(_));
	sch_initialize();

	/**
	 * @<b>scenario<\b>: Sequence of correct task added - to reach max task list size.
	 * @<b>expected<\b>: All tasks added correctly.
	 */

	EXPECT_EQ(items_list.size, 0);
	EXPECT_EQ(items_list.capacity, DEFAULT_TASKLIST_SIZE);

	for (uint8_t i = 0; i < DEFAULT_TASKLIST_SIZE; i++)
	{
		EXPECT_EQ(RETURN_OK, sch_subscribe(&fake_callback1));
	}

	EXPECT_EQ(items_list.size, DEFAULT_TASKLIST_SIZE);
	EXPECT_EQ(items_list.capacity, DEFAULT_TASKLIST_SIZE);

	/**
	 * @<b>scenario<\b>: List is full, task is added.
	 * @<b>expected<\b>: List should be reallocated and resized with DEFAULT_TASK_INCR step.
	 */
	EXPECT_EQ(RETURN_OK, sch_subscribe(&fake_callback1));
	EXPECT_EQ(items_list.size, DEFAULT_TASKLIST_SIZE + 1);
	EXPECT_EQ(items_list.capacity, DEFAULT_TASKLIST_SIZE + DEFAULT_TASK_INCR);

	EXPECT_CALL(*time_cnt_mock, time_unregister_callback(_));
	sch_deinitialize();
}

/**
 * @test Task scheduling test
 */
TEST_F(timeFixture, task_schedule_test_one_task)
{

	TimeItem item = {};

	EXPECT_CALL(*time_cnt_mock, time_register_callback(_));
	EXPECT_CALL(*time_cnt_mock, time_get_basetime()).WillRepeatedly(Return(10));
	sch_initialize();

	/**
	 * @<b>scenario<\b>: One task added - enabled, period 30ms.
	 * @<b>expected<\b>: Task added correctly.
	 */
	EXPECT_EQ(RETURN_OK, sch_subscribe(&fake_callback1));
	EXPECT_EQ(RETURN_OK, sch_set_task_state(&fake_callback1, TASKSTATE_RUNNING));
	EXPECT_EQ(RETURN_OK, sch_set_task_type(&fake_callback1, TASKTYPE_PERIODIC));
	EXPECT_EQ(RETURN_OK, sch_set_task_period(&fake_callback1, 30));

	EXPECT_CALL(*callMock, task1_callback()).Times(3);

	for (uint8_t i = 0; i < 9; i++)
	{
		sch_on_time_change(&item);
		sch_task_watcher();
	}

	EXPECT_EQ(30, sch_get_task_period(&fake_callback1));
	EXPECT_EQ(TASKTYPE_PERIODIC, sch_get_task_type(&fake_callback1));
	EXPECT_EQ(TASKSTATE_RUNNING, sch_get_task_state(&fake_callback1));

	/**
	 * @<b>scenario<\b>: Task disabled.
	 * @<b>expected<\b>: Task not called.
	 */
	EXPECT_EQ(RETURN_OK, sch_set_task_state(&fake_callback1, TASKSTATE_STOPPED));

	EXPECT_CALL(*callMock, task1_callback()).Times(0);

	for (uint8_t i = 0; i < 9; i++)
	{
		sch_on_time_change(&item);
		sch_task_watcher();
	}

	EXPECT_EQ(TASKSTATE_STOPPED, sch_get_task_state(&fake_callback1));

	/**
	 * @<b>scenario<\b>: Task enabled, period changed.
	 * @<b>expected<\b>: Task called with new period.
	 */

	EXPECT_EQ(RETURN_OK, sch_set_task_state(&fake_callback1, TASKSTATE_RUNNING));
	EXPECT_EQ(RETURN_OK, sch_set_task_period(&fake_callback1, 100));

	EXPECT_CALL(*callMock, task1_callback()).Times(3);

	for (uint8_t i = 0; i < 30; i++)
	{
		sch_on_time_change(&item);
		sch_task_watcher();
	}
	EXPECT_EQ(100, sch_get_task_period(&fake_callback1));
	EXPECT_EQ(TASKSTATE_RUNNING, sch_get_task_state(&fake_callback1));

	/**
	 * @<b>scenario<\b>: Task type set to once.
	 * @<b>expected<\b>: Task called only once.
	 */

	EXPECT_EQ(RETURN_OK, sch_set_task_type(&fake_callback1, TASKTYPE_TRIGGER));

	EXPECT_CALL(*callMock, task1_callback()).Times(1);

	for (uint8_t i = 0; i < 30; i++)
	{
		sch_on_time_change(&item);
		sch_task_watcher();
	}
	EXPECT_EQ(TASKTYPE_TRIGGER, sch_get_task_type(&fake_callback1));
	EXPECT_EQ(TASKSTATE_STOPPED, sch_get_task_state(&fake_callback1));
	/**
	 * @<b>scenario<\b>: Task type is set to once, task triggerred.
	 * @<b>expected<\b>: Task called after timeout.
	 */

	EXPECT_EQ(RETURN_OK, sch_trigger_task(&fake_callback1));
	EXPECT_EQ(TASKTYPE_TRIGGER, sch_get_task_type(&fake_callback1));
	EXPECT_EQ(TASKSTATE_RUNNING, sch_get_task_state(&fake_callback1));

	EXPECT_CALL(*callMock, task1_callback()).Times(1);

	for (uint8_t i = 0; i < 30; i++)
	{
		sch_on_time_change(&item);
		sch_task_watcher();
	}

	EXPECT_EQ(TASKTYPE_TRIGGER, sch_get_task_type(&fake_callback1));
	EXPECT_EQ(TASKSTATE_STOPPED, sch_get_task_state(&fake_callback1));

	EXPECT_CALL(*time_cnt_mock, time_unregister_callback(_));
	sch_deinitialize();
}

/**
 * @test Task scheduling test when more than one task added
 */
TEST_F(timeFixture, three_tasks_handling)
{
	TimeItem item = {};

	EXPECT_CALL(*time_cnt_mock, time_register_callback(_));
	EXPECT_CALL(*time_cnt_mock, time_get_basetime()).WillRepeatedly(Return(10));
	sch_initialize();

	/**
	 * @<b>scenario<\b>: Three tasks active with different period.
	 * @<b>expected<\b>: Tasks called in correct sequence and timing.
	 */
	EXPECT_EQ(RETURN_OK, sch_subscribe(&fake_callback1));
	EXPECT_EQ(RETURN_OK, sch_subscribe(&fake_callback2));
	EXPECT_EQ(RETURN_OK, sch_subscribe(&fake_callback3));

	EXPECT_EQ(RETURN_OK, sch_set_task_state(&fake_callback1, TASKSTATE_RUNNING));
	EXPECT_EQ(RETURN_OK, sch_set_task_type(&fake_callback1, TASKTYPE_PERIODIC));
	EXPECT_EQ(RETURN_OK, sch_set_task_period(&fake_callback1, 10));
	EXPECT_EQ(RETURN_OK, sch_set_task_state(&fake_callback2, TASKSTATE_RUNNING));
	EXPECT_EQ(RETURN_OK, sch_set_task_type(&fake_callback2, TASKTYPE_PERIODIC));
	EXPECT_EQ(RETURN_OK, sch_set_task_period(&fake_callback2, 20));
	EXPECT_EQ(RETURN_OK, sch_set_task_state(&fake_callback3, TASKSTATE_RUNNING));
	EXPECT_EQ(RETURN_OK, sch_set_task_type(&fake_callback3, TASKTYPE_PERIODIC));
	EXPECT_EQ(RETURN_OK, sch_set_task_period(&fake_callback3, 40));

	EXPECT_CALL(*callMock, task1_callback()).Times(8);
	EXPECT_CALL(*callMock, task2_callback()).Times(4);
	EXPECT_CALL(*callMock, task3_callback()).Times(2);

	for (uint8_t i = 0; i < 8; i++)
	{
		sch_on_time_change(&item);
		sch_task_watcher();
	}

	/**
	 * @<b>scenario<\b>: Second task period changed.
	 * @<b>expected<\b>: Tasks called in correct sequence and timing.
	 */

	EXPECT_EQ(RETURN_OK, sch_set_task_period(&fake_callback2, 10));

	EXPECT_CALL(*callMock, task1_callback()).Times(8);
	EXPECT_CALL(*callMock, task2_callback()).Times(8);
	EXPECT_CALL(*callMock, task3_callback()).Times(2);

	for (uint8_t i = 0; i < 8; i++)
	{
		sch_on_time_change(&item);
		sch_task_watcher();
	}

	/**
	 * @<b>scenario<\b>: First task set to once.
	 * @<b>expected<\b>: Tasks called in correct sequence and timing.
	 */

	EXPECT_EQ(RETURN_OK, sch_set_task_type(&fake_callback1, TASKTYPE_TRIGGER));

	EXPECT_CALL(*callMock, task1_callback()).Times(1);
	EXPECT_CALL(*callMock, task2_callback()).Times(8);
	EXPECT_CALL(*callMock, task3_callback()).Times(2);

	for (uint8_t i = 0; i < 8; i++)
	{
		sch_on_time_change(&item);
		sch_task_watcher();
	}

	/**
	 * @<b>scenario<\b>: First task of type once retrigerred.
	 * @<b>expected<\b>: Tasks called in correct sequence and timing.
	 */

	EXPECT_EQ(RETURN_OK, sch_trigger_task(&fake_callback1));


	EXPECT_CALL(*callMock, task1_callback()).Times(1);
	EXPECT_CALL(*callMock, task2_callback()).Times(8);
	EXPECT_CALL(*callMock, task3_callback()).Times(2);

	for (uint8_t i = 0; i < 8; i++)
	{
		sch_on_time_change(&item);
		sch_task_watcher();
	}

	/**
	 * @<b>scenario<\b>: Second task removed.
	 * @<b>expected<\b>: Tasks called in correct sequence and timing.
	 */

	EXPECT_EQ(RETURN_OK, sch_set_task_type(&fake_callback1, TASKTYPE_PERIODIC));
	EXPECT_EQ(RETURN_OK, sch_unsubscribe(&fake_callback2));


	EXPECT_CALL(*callMock, task1_callback()).Times(8);
	EXPECT_CALL(*callMock, task2_callback()).Times(0);
	EXPECT_CALL(*callMock, task3_callback()).Times(2);

	for (uint8_t i = 0; i < 8; i++)
	{
		sch_on_time_change(&item);
		sch_task_watcher();
	}

	/**
	 * @<b>scenario<\b>: Third task disabled.
	 * @<b>expected<\b>: Tasks called in correct sequence and timing.
	 */

	EXPECT_EQ(RETURN_OK, sch_set_task_state(&fake_callback3, TASKSTATE_STOPPED));


	EXPECT_CALL(*callMock, task1_callback()).Times(8);
	EXPECT_CALL(*callMock, task2_callback()).Times(0);
	EXPECT_CALL(*callMock, task3_callback()).Times(0);

	for (uint8_t i = 0; i < 8; i++)
	{
		sch_on_time_change(&item);
		sch_task_watcher();
	}

	/**
	 * @<b>scenario<\b>: Third task enabled again.
	 * @<b>expected<\b>: Tasks called in correct sequence and timing.
	 */

	EXPECT_EQ(RETURN_OK, sch_set_task_state(&fake_callback3, TASKSTATE_RUNNING));


	EXPECT_CALL(*callMock, task1_callback()).Times(8);
	EXPECT_CALL(*callMock, task2_callback()).Times(0);
	EXPECT_CALL(*callMock, task3_callback()).Times(2);

	for (uint8_t i = 0; i < 8; i++)
	{
		sch_on_time_change(&item);
		sch_task_watcher();
	}

	EXPECT_CALL(*time_cnt_mock, time_unregister_callback(_));
	sch_deinitialize();
}


TEST_F(timeFixture, dynamic_tasks_handling)
{
TimeItem item = {};

	EXPECT_CALL(*time_cnt_mock, time_register_callback(_));
	EXPECT_CALL(*time_cnt_mock, time_get_basetime()).WillRepeatedly(Return(10));
	sch_initialize();

	/**
	 * @<b>scenario<\b>: Three tasks active with different period.
	 * @<b>expected<\b>: Tasks called in correct sequence and timing.
	 */
	EXPECT_EQ(RETURN_OK, sch_subscribe(&fake_callback1));
	EXPECT_EQ(RETURN_OK, sch_subscribe(&fake_callback2));

	EXPECT_EQ(RETURN_OK, sch_set_task_state(&fake_callback1, TASKSTATE_RUNNING));
	EXPECT_EQ(RETURN_OK, sch_set_task_type(&fake_callback1, TASKTYPE_PERIODIC));
	EXPECT_EQ(RETURN_OK, sch_set_task_period(&fake_callback1, 10));
	EXPECT_EQ(RETURN_OK, sch_set_task_state(&fake_callback2, TASKSTATE_RUNNING));
	EXPECT_EQ(RETURN_OK, sch_set_task_type(&fake_callback2, TASKTYPE_PERIODIC));
	EXPECT_EQ(RETURN_OK, sch_set_task_period(&fake_callback2, 20));

	EXPECT_CALL(*callMock, task1_callback()).Times(8);
	EXPECT_CALL(*callMock, task2_callback()).Times(4);

	EXPECT_EQ(items_list.size, 2);

	for (uint8_t i = 0; i < 8; i++)
	{
		sch_on_time_change(&item);
		sch_task_watcher();
	}

	/**
	 * @<b>scenario<\b>: Dynamic task added.
	 * @<b>expected<\b>: Tasks called in correct sequence and timing.
	 */
	EXPECT_EQ(RETURN_OK, sch_schedule_task(&fake_callback3, 20));
	EXPECT_EQ(items_list.size, 3);


	EXPECT_CALL(*callMock, task1_callback()).Times(8);
	EXPECT_CALL(*callMock, task2_callback()).Times(4);
	EXPECT_CALL(*callMock, task3_callback()).Times(1);

	for (uint8_t i = 0; i < 8; i++)
	{
		sch_on_time_change(&item);
		sch_task_watcher();
	}

	EXPECT_EQ(items_list.size, 2);

	EXPECT_CALL(*time_cnt_mock, time_unregister_callback(_));
	sch_deinitialize();

}














//negative cases


