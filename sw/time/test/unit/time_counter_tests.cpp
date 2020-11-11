#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/time_counter.c"
#include "../../../../ext_lib/CMSIS/stubs/device/stm32f4xx.h"
#ifdef __cplusplus
}
#endif


/**
 * @brief Unit test of time module.
 *
 * All tests that verify behavior of time module
 *
 * @file time_counter_tests.cpp
 * @author  Jacek Skowronek
 * @date    06/11/2020
 */


using namespace ::testing;

struct callbackMock
{
	MOCK_METHOD1(callback, void(TimeItem*));
};

callbackMock* callMock;

void fake_callback(TimeItem* item)
{
	callMock->callback(item);
}

struct timeFixture : public ::testing::Test
{
	virtual void SetUp()
	{
		stm_stub_init();
		callMock = new callbackMock();
	}

	virtual void TearDown()
	{
		stm_stub_deinit();
		delete callMock;
	}
};

/**
 * @test Time incrementing test
 */
TEST_F(timeFixture, time_increment_test)
{
	time_init();
	/**
	 * @<b>scenario<\b>: Module initialized, interrupt fired.
	 * @<b>expected<\b>: msecond incremented.
	 */

	/* interrupt fired */
	SysTick_Handler();
	time_watcher();

	TimeItem t;
	EXPECT_EQ(RETURN_OK, time_get(&t));

	EXPECT_EQ(t.day, 1);
	EXPECT_EQ(t.month, 1);
	EXPECT_EQ(t.year, 2000);
	EXPECT_EQ(t.hour, 0);
	EXPECT_EQ(t.minute, 0);
	EXPECT_EQ(t.second, 0);
	EXPECT_EQ(t.msecond, 10);

	/**
	 * @<b>scenario<\b>: Switching from msecond to seconds.
	 * @<b>expected<\b>: Correct time.
	 */

	for (uint8_t i = 0; i < 99; i++)
	{
		SysTick_Handler();
		time_watcher();
	}

	EXPECT_EQ(RETURN_OK, time_get(&t));
	EXPECT_EQ(t.day, 1);
	EXPECT_EQ(t.month, 1);
	EXPECT_EQ(t.year, 2000);
	EXPECT_EQ(t.hour, 0);
	EXPECT_EQ(t.minute, 0);
	EXPECT_EQ(t.second, 1);
	EXPECT_EQ(t.msecond, 0);

	/**
	 * @<b>scenario<\b>: Switching from seconds to minutes.
	 * @<b>expected<\b>: Correct time.
	 */

	t.second = 59;
	t.msecond = 990;

	time_set(&t);

	SysTick_Handler();
	time_watcher();

	EXPECT_EQ(RETURN_OK, time_get(&t));
	EXPECT_EQ(t.day, 1);
	EXPECT_EQ(t.month, 1);
	EXPECT_EQ(t.year, 2000);
	EXPECT_EQ(t.hour, 0);
	EXPECT_EQ(t.minute, 1);
	EXPECT_EQ(t.second, 0);
	EXPECT_EQ(t.msecond, 0);

	/**
	 * @<b>scenario<\b>: Switching from minutes to hours.
	 * @<b>expected<\b>: Correct time.
	 */

	t.minute = 59;
	t.second = 59;
	t.msecond = 990;

	time_set(&t);

	SysTick_Handler();
	time_watcher();

	EXPECT_EQ(RETURN_OK, time_get(&t));
	EXPECT_EQ(t.day, 1);
	EXPECT_EQ(t.month, 1);
	EXPECT_EQ(t.year, 2000);
	EXPECT_EQ(t.hour, 1);
	EXPECT_EQ(t.minute, 0);
	EXPECT_EQ(t.second, 0);
	EXPECT_EQ(t.msecond, 0);

	/**
	 * @<b>scenario<\b>: Switching from hours to days.
	 * @<b>expected<\b>: Correct time.
	 */

	t.hour = 23;
	t.minute = 59;
	t.second = 59;
	t.msecond = 990;

	time_set(&t);

	SysTick_Handler();
	time_watcher();

	EXPECT_EQ(RETURN_OK, time_get(&t));
	EXPECT_EQ(t.day, 2);
	EXPECT_EQ(t.month, 1);
	EXPECT_EQ(t.year, 2000);
	EXPECT_EQ(t.hour, 0);
	EXPECT_EQ(t.minute, 0);
	EXPECT_EQ(t.second, 0);
	EXPECT_EQ(t.msecond, 0);

	/**
	 * @<b>scenario<\b>: Switching from days to months.
	 * @<b>expected<\b>: Correct time.
	 */

	t.day = 31;
	t.month = 1;
	t.hour = 23;
	t.minute = 59;
	t.second = 59;
	t.msecond = 990;

	time_set(&t);

	SysTick_Handler();
	time_watcher();

	EXPECT_EQ(RETURN_OK, time_get(&t));
	EXPECT_EQ(t.day, 1);
	EXPECT_EQ(t.month, 2);
	EXPECT_EQ(t.year, 2000);
	EXPECT_EQ(t.hour, 0);
	EXPECT_EQ(t.minute, 0);
	EXPECT_EQ(t.second, 0);
	EXPECT_EQ(t.msecond, 0);

	/**
	 * @<b>scenario<\b>: Switching from months to years.
	 * @<b>expected<\b>: Correct time.
	 */

	t.day = 31;
	t.month = 12;
	t.hour = 23;
	t.minute = 59;
	t.second = 59;
	t.msecond = 990;

	time_set(&t);

	SysTick_Handler();
	time_watcher();

	EXPECT_EQ(RETURN_OK, time_get(&t));
	EXPECT_EQ(t.day, 1);
	EXPECT_EQ(t.month, 1);
	EXPECT_EQ(t.year, 2001);
	EXPECT_EQ(t.hour, 0);
	EXPECT_EQ(t.minute, 0);
	EXPECT_EQ(t.second, 0);
	EXPECT_EQ(t.msecond, 0);

	/**
	 * @<b>scenario<\b>: Switching from days to month - non-leap year.
	 * @<b>expected<\b>: Correct time.
	 */

	t.day = 28;
	t.month = 2;
	t.year = 2019;
	t.hour = 23;
	t.minute = 59;
	t.second = 59;
	t.msecond = 990;

	time_set(&t);

	SysTick_Handler();
	time_watcher();

	EXPECT_EQ(RETURN_OK, time_get(&t));
	EXPECT_EQ(t.day, 1);
	EXPECT_EQ(t.month, 3);
	EXPECT_EQ(t.year, 2019);
	EXPECT_EQ(t.hour, 0);
	EXPECT_EQ(t.minute, 0);
	EXPECT_EQ(t.second, 0);
	EXPECT_EQ(t.msecond, 0);
}

TEST_F(timeFixture, leap_year_tests)
{
	uint16_t non_leap_years[] = {2001, 2002, 2003, 2005, 2006, 2006, 2009, 2010, 2011, 2013, 2014, 2015, 2017, 2018, 2019, 2021, 2022, 2023};
	uint16_t leap_years[] = {2000, 2004, 2008, 2012, 2016, 2020};
	/**
	 * @<b>scenario<\b>: Non-leap years checked
	 * @<b>expected<\b>: Correct leap-ness.
	 */
	for (auto year : non_leap_years)
	{
		timestamp.year = year;
		EXPECT_EQ(RETURN_NOK, is_leap_year());
	}

	/**
	 * @<b>scenario<\b>: Leap years checked
	 * @<b>expected<\b>: Correct leap-ness.
	 */
	for (auto year : leap_years)
	{
		timestamp.year = year;
		EXPECT_EQ(RETURN_OK, is_leap_year());
	}
}

/**
 * @test Callback call tests
 */
TEST_F(timeFixture, callback_tests)
{
	time_init();
	TimeItem t;
	/**
	 * @<b>scenario<\b>: Time changed when any callback is registered.
	 * @<b>expected<\b>: Callback should be called.
	 */

	EXPECT_EQ(RETURN_OK, time_register_callback(&fake_callback));
	t.day = 20;
	t.month = 12;
	t.year = 2020;
	t.hour = 12;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	time_set(&t);

	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](TimeItem* item)
			{
				if (!item) return;
				EXPECT_EQ(item->day, 20);
				EXPECT_EQ(item->month, 12);
				EXPECT_EQ(item->year, 2020);
				EXPECT_EQ(item->hour, 12);
				EXPECT_EQ(item->second, 30);
				EXPECT_EQ(item->msecond, 40);
			}));

	SysTick_Handler();
	time_watcher();
	EXPECT_EQ(RETURN_OK, time_unregister_callback(&fake_callback));


	/**
	 * @<b>scenario<\b>: Time changed when all callbacks is registered.
	 * @<b>expected<\b>: Callback should be called.
	 */

	for (uint8_t i = 0; i < TIME_CNT_CALLBACK_MAX_SIZE; i++)
	{
		EXPECT_EQ(RETURN_OK, time_register_callback(&fake_callback));
	}

	EXPECT_EQ(RETURN_NOK, time_register_callback(&fake_callback));

	t.day = 20;
	t.month = 12;
	t.year = 2020;
	t.hour = 12;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	time_set(&t);

	EXPECT_CALL(*callMock, callback(_)).Times(TIME_CNT_CALLBACK_MAX_SIZE).WillRepeatedly(Invoke([&](TimeItem* item)
			{
				if (!item) return;
				EXPECT_EQ(item->day, 20);
				EXPECT_EQ(item->month, 12);
				EXPECT_EQ(item->year, 2020);
				EXPECT_EQ(item->hour, 12);
				EXPECT_EQ(item->second, 30);
				EXPECT_EQ(item->msecond, 40);
			}));

	SysTick_Handler();
	time_watcher();
}

/**
 * @test Setting/Getting time test
 */
TEST_F(timeFixture, set_get_time)
{
	TimeItem t;

	/**
	 * @<b>scenario<\b>: Setting time with NULL.
	 * @<b>expected<\b>: False returned
	 */

	EXPECT_EQ(RETURN_NOK, time_set(NULL));

	/**
	 * @<b>scenario<\b>: Getting time with NULL.
	 * @<b>expected<\b>: False returned
	 */

	EXPECT_EQ(RETURN_NOK, time_get(NULL));

	/**
	 * @<b>scenario<\b>: Setting time with incorrect day.
	 * @<b>expected<\b>: False returned
	 */

	t.day = 32;
	t.month = 1;
	t.year = 2020;
	t.hour = 12;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_NOK, time_set(&t));

	/**
	 * @<b>scenario<\b>: Setting time with correct day.
	 * @<b>expected<\b>: True returned
	 */

	t.day = 31;
	t.month = 1;
	t.year = 2020;
	t.hour = 12;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_OK, time_set(&t));

	/**
	 * @<b>scenario<\b>: Setting time with incorrect month.
	 * @<b>expected<\b>: False returned
	 */

	t.day = 30;
	t.month = 13;
	t.year = 2020;
	t.hour = 12;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_NOK, time_set(&t));

	/**
	 * @<b>scenario<\b>: Setting time with correct month.
	 * @<b>expected<\b>: True returned
	 */

	t.day = 30;
	t.month = 12;
	t.year = 2020;
	t.hour = 12;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_OK, time_set(&t));

	/**
	 * @<b>scenario<\b>: Setting time with incorrect hour.
	 * @<b>expected<\b>: False returned
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 24;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_NOK, time_set(&t));

	/**
	 * @<b>scenario<\b>: Setting time with correct hour.
	 * @<b>expected<\b>: True returned
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_OK, time_set(&t));

	/**
	 * @<b>scenario<\b>: Setting time with incorrect minute.
	 * @<b>expected<\b>: False returned
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 60;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_NOK, time_set(&t));

	/**
	 * @<b>scenario<\b>: Setting time with correct minute.
	 * @<b>expected<\b>: True returned
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 59;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_OK, time_set(&t));

	/**
	 * @<b>scenario<\b>: Setting time with incorrect second.
	 * @<b>expected<\b>: False returned
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 59;
	t.second = 60;
	t.msecond = 30;

	EXPECT_EQ(RETURN_NOK, time_set(&t));

	/**
	 * @<b>scenario<\b>: Setting time with correct second.
	 * @<b>expected<\b>: True returned
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 59;
	t.second = 59;
	t.msecond = 30;

	EXPECT_EQ(RETURN_OK, time_set(&t));

	/**
	 * @<b>scenario<\b>: Setting time with incorrect mseconds.
	 * @<b>expected<\b>: False returned
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 59;
	t.second = 59;
	t.msecond = 1000;

	EXPECT_EQ(RETURN_NOK, time_set(&t));

	/**
	 * @<b>scenario<\b>: Setting time with incorrect mseconds.
	 * @<b>expected<\b>: False returned
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 59;
	t.second = 59;
	t.msecond = 54;

	EXPECT_EQ(RETURN_NOK, time_set(&t));

	/**
	 * @<b>scenario<\b>: Setting time with correct mseconds.
	 * @<b>expected<\b>: True returned
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 59;
	t.second = 59;
	t.msecond = 90;

	EXPECT_EQ(RETURN_OK, time_set(&t));
}
