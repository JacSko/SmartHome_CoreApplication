#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/task_scheduler.c"
#ifdef __cplusplus
}
#endif

#include "time_counter_mock.h"
#include "logger_mock.h"

/* ============================= */
/**
 * @file task_scheduler_tests.cpp
 *
 * @brief Unit tests of Task Scheduler module
 *
 * @details
 * This tests verifies behavior of Task Scheduler module
 *
 * @author Jacek Skowronek
 * @date 01/11/2020
 */
/* ============================= */

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
		mock_logger_init();
		callMock = new callbackMock();
	}

	virtual void TearDown()
	{
		mock_time_counter_deinit();
		mock_logger_deinit();
		delete callMock;
	}
};

/**
 * @test Module initialization, adding simple tasks
 */
TEST_F(timeFixture, module_init_task_adding)
{
	/**
	 * <b>scenario</b>: Module initialisation.<br>
	 * <b>expected</b>: Callback registered.<br>
    * ************************************************
	 */
	EXPECT_CALL(*time_cnt_mock, time_register_callback(_,TIME_PRIORITY_HIGH));
	sch_initialize();

	/**
	 * <b>scenario</b>: First task added.<br>
	 * <b>expected</b>: Task added correctly.<br>
    * ************************************************
	 */
	EXPECT_CALL(*time_cnt_mock, time_get_basetime()).WillOnce(Return(10));
	EXPECT_EQ(RETURN_OK, sch_subscribe(&fake_callback1));
	EXPECT_EQ(RETURN_OK, sch_set_task_state(&fake_callback1, TASKSTATE_RUNNING));
	EXPECT_EQ(RETURN_OK, sch_set_task_period(&fake_callback1, 1000));
	EXPECT_EQ(items_list.size, 1);

	/**
	 * <b>scenario</b>: First task removed.<br>
	 * <b>expected</b>: Task removed correctly.<br>
    * ************************************************
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
	EXPECT_CALL(*time_cnt_mock, time_register_callback(_,TIME_PRIORITY_HIGH));
	sch_initialize();

	/**
	 * <b>scenario</b>: Sequence of correct task added - to reach max task list size.<br>
	 * <b>expected</b>: All tasks added correctly.<br>
    * ************************************************
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
	 * <b>scenario</b>: List is full, task is added.<br>
	 * <b>expected</b>: List should be reallocated and resized with DEFAULT_TASK_INCR step.<br>
    * ************************************************
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

	EXPECT_CALL(*time_cnt_mock, time_register_callback(_,TIME_PRIORITY_HIGH));
	EXPECT_CALL(*time_cnt_mock, time_get_basetime()).WillRepeatedly(Return(10));
	sch_initialize();

	/**
	 * <b>scenario</b>: One task added - enabled, period 30ms.<br>
	 * <b>expected</b>: Task added correctly.<br>
    * ************************************************
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
	 * <b>scenario</b>: Task disabled.<br>
	 * <b>expected</b>: Task not called.<br>
    * ************************************************
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
	 * <b>scenario</b>: Task enabled, period changed.<br>
	 * <b>expected</b>: Task called with new period.<br>
    * ************************************************
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
	 * <b>scenario</b>: Task type set to once.<br>
	 * <b>expected</b>: Task called only once.<br>
    * ************************************************
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
	 * <b>scenario</b>: Task type is set to once, task triggerred.<br>
	 * <b>expected</b>: Task called after timeout.<br>
    * ************************************************
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

	EXPECT_CALL(*time_cnt_mock, time_register_callback(_,TIME_PRIORITY_HIGH));
	EXPECT_CALL(*time_cnt_mock, time_get_basetime()).WillRepeatedly(Return(10));
	sch_initialize();

	/**
	 * <b>scenario</b>: Three tasks active with different period.<br>
	 * <b>expected</b>: Tasks called in correct sequence and timing.<br>
    * ************************************************
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
	 * <b>scenario</b>: Second task period changed.<br>
	 * <b>expected</b>: Tasks called in correct sequence and timing.<br>
    * ************************************************
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
	 * <b>scenario</b>: First task set to once.<br>
	 * <b>expected</b>: Tasks called in correct sequence and timing.<br>
    * ************************************************
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
	 * <b>scenario</b>: First task of type once retrigerred.<br>
	 * <b>expected</b>: Tasks called in correct sequence and timing.<br>
    * ************************************************
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
	 * <b>scenario</b>: Second task removed.<br>
	 * <b>expected</b>: Tasks called in correct sequence and timing.<br>
    * ************************************************
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
	 * <b>scenario</b>: Third task disabled.<br>
	 * <b>expected</b>: Tasks called in correct sequence and timing.<br>
    * ************************************************
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
	 * <b>scenario</b>: Third task enabled again.<br>
	 * <b>expected</b>: Tasks called in correct sequence and timing.<br>
    * ************************************************
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

/**
 * @test Dynamic task handling
 */
TEST_F(timeFixture, dynamic_tasks_handling)
{
	TimeItem item = {};

	EXPECT_CALL(*time_cnt_mock, time_register_callback(_,TIME_PRIORITY_HIGH));
	EXPECT_CALL(*time_cnt_mock, time_get_basetime()).WillRepeatedly(Return(10));
	sch_initialize();

	/**
	 * <b>scenario</b>: Three tasks active with different period.<br>
	 * <b>expected</b>: Tasks called in correct sequence and timing.<br>
    * ************************************************
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
	 * <b>scenario</b>: Dynamic task added.<br>
	 * <b>expected</b>: Tasks called in correct sequence and timing.<br>
    * ************************************************
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
/**
 * @test Negative cases for module
 */
TEST_F(timeFixture, negative_cases)
{

	EXPECT_CALL(*time_cnt_mock, time_register_callback(_,TIME_PRIORITY_HIGH));
	EXPECT_CALL(*time_cnt_mock, time_get_basetime()).WillRepeatedly(Return(10));
	sch_initialize();

	/**
	 * <b>scenario</b>: Subscribe invalid task.<br>
	 * <b>expected</b>: False returned.<br>
    * ************************************************
	 */

	EXPECT_EQ(RETURN_NOK, sch_subscribe(NULL));

	/**
	 * <b>scenario</b>: Unsubscribe not existing task.<br>
	 * <b>expected</b>: False returned.<br>
    * ************************************************
	 */

	EXPECT_EQ(RETURN_NOK, sch_unsubscribe(&fake_callback1));

	/**
	 * <b>scenario</b>: Schedule task with invalid period.<br>
	 * <b>expected</b>: False returned.<br>
    * ************************************************
	 */

	EXPECT_EQ(RETURN_NOK, sch_schedule_task(&fake_callback2, 1));
	EXPECT_EQ(items_list.size, 0);

	/**
	 * <b>scenario</b>: Set/Get period for not existing task.<br>
	 * <b>expected</b>: False returned.<br>
    * ************************************************
	 */

	EXPECT_EQ(RETURN_NOK, sch_set_task_period(&fake_callback1, 100));
	EXPECT_EQ(0, sch_get_task_period(&fake_callback1));

	/**
	 * <b>scenario</b>: Set/Get task state for not existing task.<br>
	 * <b>expected</b>: False returned.<br>
    * ************************************************
	 */

	EXPECT_EQ(RETURN_NOK, sch_set_task_state(&fake_callback1, TASKSTATE_RUNNING));
	EXPECT_EQ(TASKSTATE_UNKNOWN, sch_get_task_state(&fake_callback1));

	/**
	 * <b>scenario</b>: Set incorrect task state for existing task.<br>
	 * <b>expected</b>: False returned.<br>
    * ************************************************
	 */

	EXPECT_EQ(RETURN_OK, sch_subscribe(&fake_callback1));
	EXPECT_EQ(RETURN_NOK, sch_set_task_state(&fake_callback1, TASKSTATE_UNKNOWN));
	EXPECT_EQ(RETURN_OK, sch_unsubscribe(&fake_callback1));

	/**
	 * <b>scenario</b>: Set/Get task type for non existing task.<br>
	 * <b>expected</b>: False returned.<br>
    * ************************************************
	 */

	EXPECT_EQ(RETURN_NOK, sch_set_task_type(&fake_callback1, TASKTYPE_PERIODIC));
	EXPECT_EQ(TASKTYPE_UNKNOWN, sch_get_task_type(&fake_callback1));

	sch_deinitialize();
}

/**
 * @test Subscribing tasks with setting all properties in the same time
 */
TEST_F(timeFixture, task_subscribe_and_set_tests)
{

   EXPECT_CALL(*time_cnt_mock, time_register_callback(_,TIME_PRIORITY_HIGH));
   EXPECT_CALL(*time_cnt_mock, time_get_basetime()).WillRepeatedly(Return(10));
   sch_initialize();

   /**
    * <b>scenario</b>: Wrong task priority.<br>
    * <b>expected</b>: Task not added.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, sch_subscribe_and_set(&fake_callback1, TASKPRIO_UNKNOWN, 1,
                        TASKSTATE_UNKNOWN, TASKTYPE_UNKNOWN));

   /**
    * <b>scenario</b>: Wrong task period.<br>
    * <b>expected</b>: Task not added.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, sch_subscribe_and_set(&fake_callback1, TASKPRIO_LOW, 1,
                        TASKSTATE_UNKNOWN, TASKTYPE_UNKNOWN));

   /**
    * <b>scenario</b>: Wrong task state.<br>
    * <b>expected</b>: Task not added.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, sch_subscribe_and_set(&fake_callback1, TASKPRIO_LOW, 1000,
                        TASKSTATE_UNKNOWN, TASKTYPE_UNKNOWN));
   /**
    * <b>scenario</b>: Wrong task type.<br>
    * <b>expected</b>: Task not added.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, sch_subscribe_and_set(&fake_callback1, TASKPRIO_LOW, 1000,
                        TASKSTATE_RUNNING, TASKTYPE_UNKNOWN));
   /**
    * <b>scenario</b>: All data correct.<br>
    * <b>expected</b>: Task added.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_OK, sch_subscribe_and_set(&fake_callback1, TASKPRIO_LOW, 1000,
                        TASKSTATE_RUNNING, TASKTYPE_PERIODIC));
   sch_deinitialize();
}

/**
 * @test Low and High priority task calling
 */
TEST_F(timeFixture, task_low_high_priority_calling_test)
{
   TimeItem item = {};
   EXPECT_CALL(*time_cnt_mock, time_register_callback(_,TIME_PRIORITY_HIGH));
   EXPECT_CALL(*time_cnt_mock, time_get_basetime()).WillRepeatedly(Return(10));
   sch_initialize();

   /**
    * <b>scenario</b>: Two tasks added - one with high priority, seconds with low.<br>
    * <b>expected</b>: Task with high priority called from interrupt routine,
    *                  but low priority task called from main thread.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_OK, sch_subscribe_and_set(&fake_callback1, TASKPRIO_HIGH, 20,
                         TASKSTATE_RUNNING, TASKTYPE_PERIODIC));
   EXPECT_EQ(RETURN_OK, sch_subscribe_and_set(&fake_callback2, TASKPRIO_LOW, 20,
                         TASKSTATE_RUNNING, TASKTYPE_PERIODIC));

   EXPECT_CALL(*callMock, task1_callback()).Times(4);
   EXPECT_CALL(*callMock, task2_callback()).Times(2);

   for (uint8_t i = 0; i < 4; i++)
   { /* only fake_callback1 called */
      sch_on_time_change(&item);
   }

   for (uint8_t i = 0; i < 4; i++)
   { /* both callback called called */
      sch_on_time_change(&item);
      sch_task_watcher();
   }
   sch_deinitialize();
}

