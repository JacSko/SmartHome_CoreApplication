#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/Logger.c"
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
	MOCK_METHOD1(callback, RET_CODE(const char*));
};

callbackMock* callMock;

RET_CODE fake_callback(const char* data)
{
	return callMock->callback(data);
}

struct loggerFixture : public ::testing::Test
{
	const uint16_t  LOGGER_BUFFER_SIZE = 512;
	virtual void SetUp()
	{
		callMock = new callbackMock;
		mock_time_counter_init();
		logger_initialize(LOGGER_BUFFER_SIZE, &fake_callback);
	}

	virtual void TearDown()
	{
		delete callMock;
		mock_time_counter_deinit();
		logger_deinitialize();
	}
};

/**
 * @test Sending log over uart
 */
TEST_F(loggerFixture, send_log)
{
	EXPECT_EQ(RETURN_OK, logger_enable());
	EXPECT_EQ(RETURN_OK, logger_set_group_state(LOG_DEBUG, 1));
	TimeItem t1 = {};
	t1.day = 1; t1.month = 2, t1.year = 2020, t1.hour = 11, t1.minute = 12, t1.second = 13, t1.msecond = 400;

	/**
	 * @<b>scenario<\b>: Send debug string.
	 * @<b>expected<\b>: Callback called.
	 */
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1));
	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](const char* data) -> RET_CODE
								{
									EXPECT_STREQ(data, "[1-2-2020 11:12:13:400] - DEBUG - FILE:DATA\n");
									return RETURN_OK;
								}));
	logger_send_log(LOG_DEBUG, "FILE", "DATA");

	/**
	 * @<b>scenario<\b>: Send debug string with invalid group.
	 * @<b>expected<\b>: Callback not called.
	 */
	EXPECT_CALL(*time_cnt_mock, time_get()).Times(0);
	EXPECT_CALL(*callMock, callback(_)).Times(0);
	logger_send_log(LOG_ENUM_MAX, "FILE", "DATA");

	/**
	 * @<b>scenario<\b>: Send debug string when logger diabled.
	 * @<b>expected<\b>: Callback not called.
	 */
	logger_disable();
	EXPECT_CALL(*time_cnt_mock, time_get()).Times(0);
	EXPECT_CALL(*callMock, callback(_)).Times(0);
	logger_send_log(LOG_DEBUG, "FILE", "DATA");

}

/**
 * @test Sending conditional log over uart
 */
TEST_F(loggerFixture, send_log_conditional)
{
	EXPECT_EQ(RETURN_OK, logger_enable());
	EXPECT_EQ(RETURN_OK, logger_set_group_state(LOG_DEBUG, 1));
	TimeItem t1 = {};
	t1.day = 1; t1.month = 2, t1.year = 2020, t1.hour = 11, t1.minute = 12, t1.second = 13, t1.msecond = 400;

	/**
	 * @<b>scenario<\b>: Send debug string when condition is True.
	 * @<b>expected<\b>: Callback called.
	 */
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1));
	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](const char* data) -> RET_CODE
								{
									EXPECT_STREQ(data, "[1-2-2020 11:12:13:400] - DEBUG - FILE:DATA\n");
									return RETURN_OK;
								}));
	logger_send_if(1==1, LOG_DEBUG, "FILE", "DATA");

	/**
	 * @<b>scenario<\b>: Send debug string when condition is False.
	 * @<b>expected<\b>: Callback not called.
	 */
	EXPECT_CALL(*time_cnt_mock, time_get()).Times(0);
	EXPECT_CALL(*callMock, callback(_)).Times(0);
	logger_send_if(1!=1, LOG_ENUM_MAX, "FILE", "DATA");

	/**
	 * @<b>scenario<\b>: Send debug string with invalid group.
	 * @<b>expected<\b>: Callback not called.
	 */
	EXPECT_CALL(*time_cnt_mock, time_get()).Times(0);
	EXPECT_CALL(*callMock, callback(_)).Times(0);
	logger_send_if(1==1, LOG_ENUM_MAX, "FILE", "DATA");

	/**
	 * @<b>scenario<\b>: Send debug string when logger diabled.
	 * @<b>expected<\b>: Callback not called.
	 */
	logger_disable();
	EXPECT_CALL(*time_cnt_mock, time_get()).Times(0);
	EXPECT_CALL(*callMock, callback(_)).Times(0);
	logger_send_if(1==1, LOG_DEBUG, "FILE", "DATA");

}

/**
 * @test Sending log when group is disabled
 */
TEST_F(loggerFixture, send_log_group_disabled)
{
	EXPECT_EQ(RETURN_OK, logger_enable());
	TimeItem t1 = {};
	t1.day = 1; t1.month = 2, t1.year = 2020, t1.hour = 11, t1.minute = 12, t1.second = 13, t1.msecond = 400;

	/**
	 * @<b>scenario<\b>: Send DEBUG message when DEBUG is disabled
	 * @<b>expected<\b>: Callback called.
	 */
	EXPECT_CALL(*time_cnt_mock, time_get()).Times(0);
	EXPECT_CALL(*callMock, callback(_)).Times(0);
	logger_send_log(LOG_DEBUG, "FILE", "DATA");


	/**
	 * @<b>scenario<\b>: Send ERROR message when DEBUG is disabled
	 * @<b>expected<\b>: Callback called.
	 */
	EXPECT_EQ(RETURN_OK, logger_set_group_state(LOG_ERROR, 1));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1));
	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](const char* data) -> RET_CODE
								{
									EXPECT_STREQ(data, "[1-2-2020 11:12:13:400] - ERROR - FILE:DATA\n");
									return RETURN_OK;
								}));
	logger_send_log(LOG_ERROR, "FILE", "DATA");
}

/**
 * @test Sending conditional log when group is disabled
 */
TEST_F(loggerFixture, send_log_conditional_group_disabled)
{
	EXPECT_EQ(RETURN_OK, logger_enable());
	TimeItem t1 = {};
	t1.day = 1; t1.month = 2, t1.year = 2020, t1.hour = 11, t1.minute = 12, t1.second = 13, t1.msecond = 400;

	/**
	 * @<b>scenario<\b>: Send DEBUG message when DEBUG is disabled
	 * @<b>expected<\b>: Callback called.
	 */
	EXPECT_EQ(logger_get_group_state(LOG_DEBUG), 0);
	EXPECT_CALL(*time_cnt_mock, time_get()).Times(0);
	EXPECT_CALL(*callMock, callback(_)).Times(0);
	logger_send_if(1==1,LOG_DEBUG, "FILE", "DATA");


	/**
	 * @<b>scenario<\b>: Send ERROR message when DEBUG is disabled
	 * @<b>expected<\b>: Callback called.
	 */
	EXPECT_EQ(RETURN_OK, logger_set_group_state(LOG_ERROR, 1));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1));
	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](const char* data) -> RET_CODE
								{
									EXPECT_STREQ(data, "[1-2-2020 11:12:13:400] - ERROR - FILE:DATA\n");
									return RETURN_OK;
								}));
	logger_send_if(1==1, LOG_ERROR, "FILE", "DATA");

	/**
	 * @<b>scenario<\b>: Send WIFI_DRIVER message when DEBUG is disabled
	 * @<b>expected<\b>: Callback called.
	 */
	EXPECT_EQ(RETURN_OK, logger_set_group_state(LOG_WIFI_DRIVER, 1));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1));
	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](const char* data) -> RET_CODE
								{
									EXPECT_STREQ(data, "[1-2-2020 11:12:13:400] - WIFI_DRV - FILE:DATA\n");
									return RETURN_OK;
								}));
	logger_send_if(1==1, LOG_WIFI_DRIVER, "FILE", "DATA");

	/**
	 * @<b>scenario<\b>: Send TIME message when DEBUG is disabled
	 * @<b>expected<\b>: Callback called.
	 */
	EXPECT_EQ(RETURN_OK, logger_set_group_state(LOG_TIME, 1));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1));
	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](const char* data) -> RET_CODE
								{
									EXPECT_STREQ(data, "[1-2-2020 11:12:13:400] - TIME - FILE:DATA\n");
									return RETURN_OK;
								}));
	logger_send_if(1==1, LOG_TIME, "FILE", "DATA");

	/**
	 * @<b>scenario<\b>: Send TASK_SCHEDULER message when DEBUG is disabled
	 * @<b>expected<\b>: Callback called.
	 */
	EXPECT_EQ(RETURN_OK, logger_set_group_state(LOG_TASK_SCHEDULER, 1));
	EXPECT_CALL(*time_cnt_mock, time_get()).WillOnce(Return(&t1));
	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](const char* data) -> RET_CODE
								{
									EXPECT_STREQ(data, "[1-2-2020 11:12:13:400] - TASK_SCH - FILE:DATA\n");
									return RETURN_OK;
								}));
	logger_send_if(1==1, LOG_TASK_SCHEDULER, "FILE", "DATA");

}

/**
 * @test Setting/Getting group states
 */
TEST_F(loggerFixture, setting_getting_group_states)
{

	/**
	 * @<b>scenario<\b>: Get state of correct group
	 * @<b>expected<\b>: Correct state returned.
	 */
	EXPECT_EQ(0, logger_get_group_state(LOG_DEBUG));
	EXPECT_EQ(0, logger_get_group_state(LOG_ERROR));

	/**
	 * @<b>scenario<\b>: Get/Set state of incorrect group
	 * @<b>expected<\b>: 0xFF state returned.
	 */
	EXPECT_EQ(255, logger_get_group_state(LOG_ENUM_MAX));
	EXPECT_EQ(RETURN_NOK, logger_set_group_state(LOG_ENUM_MAX,1));

	/**
	 * @<b>scenario<\b>: Enable currently disabled group
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	EXPECT_EQ(RETURN_OK, logger_set_group_state(LOG_DEBUG,1));
	EXPECT_EQ(1, logger_get_group_state(LOG_DEBUG));

	/**
	 * @<b>scenario<\b>: Enable currently enabled group
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_EQ(RETURN_NOK, logger_set_group_state(LOG_DEBUG,1));
	EXPECT_EQ(1, logger_get_group_state(LOG_DEBUG));

	/**
	 * @<b>scenario<\b>: Disable currently enabled group
	 * @<b>expected<\b>: RETURN_OK returned.
	 */
	EXPECT_EQ(RETURN_OK, logger_set_group_state(LOG_DEBUG,0));
	EXPECT_EQ(0, logger_get_group_state(LOG_DEBUG));

	/**
	 * @<b>scenario<\b>: Disable currently disabled group
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
	EXPECT_EQ(RETURN_NOK, logger_set_group_state(LOG_DEBUG,0));
	EXPECT_EQ(0, logger_get_group_state(LOG_DEBUG));
}