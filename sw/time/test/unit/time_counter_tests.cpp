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

#include "logger_mock.h"

/* ============================= */
/**
 * @file time_counter_tests.cpp
 *
 * @brief Unit tests of Time module
 *
 * @details
 * This tests verifies behavior of Time module
 *
 * @author Jacek Skowronek
 * @date 01/11/2020
 */
/* ============================= */

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
	 * <b>scenario</b>: Module initialized, interrupt fired.<br>
	 * <b>expected</b>: msecond incremented.<br>
    * ************************************************
	 */

	/* interrupt fired */
	SysTick_Handler();
	time_watcher();

	TimeItem* t = time_get();

	EXPECT_EQ(t->day, 1);
	EXPECT_EQ(t->month, 1);
	EXPECT_EQ(t->year, 2000);
	EXPECT_EQ(t->hour, 0);
	EXPECT_EQ(t->minute, 0);
	EXPECT_EQ(t->second, 0);
	EXPECT_EQ(t->msecond, 10);

	/**
	 * <b>scenario</b>: Switching from msecond to seconds.<br>
	 * <b>expected</b>: Correct time.<br>
    * ************************************************
	 */

	for (uint8_t i = 0; i < 99; i++)
	{
		SysTick_Handler();
		time_watcher();
	}

	EXPECT_EQ(t->day, 1);
	EXPECT_EQ(t->month, 1);
	EXPECT_EQ(t->year, 2000);
	EXPECT_EQ(t->hour, 0);
	EXPECT_EQ(t->minute, 0);
	EXPECT_EQ(t->second, 1);
	EXPECT_EQ(t->msecond, 0);

	/**
	 * <b>scenario</b>: Switching from seconds to minutes.<br>
	 * <b>expected</b>: Correct time.<br>
    * ************************************************
	 */

	t->second = 59;
	t->msecond = 990;

	SysTick_Handler();
	time_watcher();

	EXPECT_EQ(t->day, 1);
	EXPECT_EQ(t->month, 1);
	EXPECT_EQ(t->year, 2000);
	EXPECT_EQ(t->hour, 0);
	EXPECT_EQ(t->minute, 1);
	EXPECT_EQ(t->second, 0);
	EXPECT_EQ(t->msecond, 0);

	/**
	 * <b>scenario</b>: Switching from minutes to hours.<br>
	 * <b>expected</b>: Correct time.<br>
    * ************************************************
	 */

	t->minute = 59;
	t->second = 59;
	t->msecond = 990;

	SysTick_Handler();
	time_watcher();

	EXPECT_EQ(t->day, 1);
	EXPECT_EQ(t->month, 1);
	EXPECT_EQ(t->year, 2000);
	EXPECT_EQ(t->hour, 1);
	EXPECT_EQ(t->minute, 0);
	EXPECT_EQ(t->second, 0);
	EXPECT_EQ(t->msecond, 0);

	/**
	 * <b>scenario</b>: Switching from hours to days.<br>
	 * <b>expected</b>: Correct time.<br>
    * ************************************************
	 */

	t->hour = 23;
	t->minute = 59;
	t->second = 59;
	t->msecond = 990;

	SysTick_Handler();
	time_watcher();

	EXPECT_EQ(t->day, 2);
	EXPECT_EQ(t->month, 1);
	EXPECT_EQ(t->year, 2000);
	EXPECT_EQ(t->hour, 0);
	EXPECT_EQ(t->minute, 0);
	EXPECT_EQ(t->second, 0);
	EXPECT_EQ(t->msecond, 0);

	/**
	 * <b>scenario</b>: Switching from days to months.<br>
	 * <b>expected</b>: Correct time.<br>
    * ************************************************
	 */

	t->day = 31;
	t->month = 1;
	t->hour = 23;
	t->minute = 59;
	t->second = 59;
	t->msecond = 990;

	SysTick_Handler();
	time_watcher();

	EXPECT_EQ(t->day, 1);
	EXPECT_EQ(t->month, 2);
	EXPECT_EQ(t->year, 2000);
	EXPECT_EQ(t->hour, 0);
	EXPECT_EQ(t->minute, 0);
	EXPECT_EQ(t->second, 0);
	EXPECT_EQ(t->msecond, 0);

	/**
	 * <b>scenario</b>: Switching from months to years.<br>
	 * <b>expected</b>: Correct time.<br>
    * ************************************************
	 */

	t->day = 31;
	t->month = 12;
	t->hour = 23;
	t->minute = 59;
	t->second = 59;
	t->msecond = 990;

	SysTick_Handler();
	time_watcher();

	EXPECT_EQ(t->day, 1);
	EXPECT_EQ(t->month, 1);
	EXPECT_EQ(t->year, 2001);
	EXPECT_EQ(t->hour, 0);
	EXPECT_EQ(t->minute, 0);
	EXPECT_EQ(t->second, 0);
	EXPECT_EQ(t->msecond, 0);

	/**
	 * <b>scenario</b>: Switching from days to month - non-leap year.<br>
	 * <b>expected</b>: Correct time.<br>
    * ************************************************
	 */

	t->day = 28;
	t->month = 2;
	t->year = 2019;
	t->hour = 23;
	t->minute = 59;
	t->second = 59;
	t->msecond = 990;

	SysTick_Handler();
	time_watcher();

	EXPECT_EQ(t->day, 1);
	EXPECT_EQ(t->month, 3);
	EXPECT_EQ(t->year, 2019);
	EXPECT_EQ(t->hour, 0);
	EXPECT_EQ(t->minute, 0);
	EXPECT_EQ(t->second, 0);
	EXPECT_EQ(t->msecond, 0);
}

TEST_F(timeFixture, leap_year_tests)
{
	uint16_t non_leap_years[] = {2001, 2002, 2003, 2005, 2006, 2006, 2009, 2010, 2011, 2013, 2014, 2015, 2017, 2018, 2019, 2021, 2022, 2023};
	uint16_t leap_years[] = {2000, 2004, 2008, 2012, 2016, 2020};
	/**
	 * <b>scenario</b>: Non-leap years checked<br>
	 * <b>expected</b>: Correct leap-ness.<br>
    * ************************************************
	 */
	for (auto year : non_leap_years)
	{
		timestamp.year = year;
		EXPECT_EQ(RETURN_NOK, time_is_leap_year());
	}

	/**
	 * <b>scenario</b>: Leap years checked<br>
	 * <b>expected</b>: Correct leap-ness.<br>
    * ************************************************
	 */
	for (auto year : leap_years)
	{
		timestamp.year = year;
		EXPECT_EQ(RETURN_OK, time_is_leap_year());
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
	 * <b>scenario</b>: Time changed when the callback is registered.<br>
	 * <b>expected</b>: Callback should be called.<br>
    * ************************************************
	 */

	EXPECT_EQ(RETURN_OK, time_register_callback(&fake_callback,TIME_PRIORITY_LOW));
	t.day = 20;
	t.month = 12;
	t.year = 2020;
	t.hour = 12;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	time_set_utc(&t);

	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](TimeItem* item)
			{
				if (!item) return;
				EXPECT_EQ(item->day, 20);
				EXPECT_EQ(item->month, 12);
				EXPECT_EQ(item->year, 2020);
				EXPECT_EQ(item->hour, 13);
				EXPECT_EQ(item->second, 30);
				EXPECT_EQ(item->msecond, 40);
			}));

	SysTick_Handler();
	time_watcher();
	EXPECT_EQ(RETURN_OK, time_unregister_callback(&fake_callback));


	/**
	 * <b>scenario</b>: Time changed when all callbacks is registered.<br>
	 * <b>expected</b>: Callback should be called.<br>
    * ************************************************
	 */

	for (uint8_t i = 0; i < TIME_CNT_CALLBACK_MAX_SIZE; i++)
	{
		EXPECT_EQ(RETURN_OK, time_register_callback(&fake_callback,TIME_PRIORITY_LOW));
	}

	EXPECT_EQ(RETURN_NOK, time_register_callback(&fake_callback,TIME_PRIORITY_LOW));

	t.day = 20;
	t.month = 12;
	t.year = 2020;
	t.hour = 12;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	time_set_utc(&t);

	EXPECT_CALL(*callMock, callback(_)).Times(TIME_CNT_CALLBACK_MAX_SIZE).WillRepeatedly(Invoke([&](TimeItem* item)
			{
				if (!item) return;
				EXPECT_EQ(item->day, 20);
				EXPECT_EQ(item->month, 12);
				EXPECT_EQ(item->year, 2020);
				EXPECT_EQ(item->hour, 13);
				EXPECT_EQ(item->second, 30);
				EXPECT_EQ(item->msecond, 40);
			}));

	SysTick_Handler();
	time_watcher();

	time_deinit();
}

/**
 * @test Setting/Getting time test
 */
TEST_F(timeFixture, set_get_time)
{
	TimeItem t;

	/**
	 * <b>scenario</b>: Setting time with NULL.<br>
	 * <b>expected</b>: False returned<br>
    * ************************************************
	 */

	EXPECT_EQ(RETURN_NOK, time_set_utc(NULL));

	/**
	 * <b>scenario</b>: Setting time with incorrect day.<br>
	 * <b>expected</b>: False returned<br>
    * ************************************************
	 */

	t.day = 32;
	t.month = 1;
	t.year = 2020;
	t.hour = 12;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_NOK, time_set_utc(&t));

	/**
	 * <b>scenario</b>: Setting time with correct day.<br>
	 * <b>expected</b>: True returned<br>
    * ************************************************
	 */

	t.day = 31;
	t.month = 1;
	t.year = 2020;
	t.hour = 12;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_OK, time_set_utc(&t));

	/**
	 * <b>scenario</b>: Setting time with incorrect month.<br>
	 * <b>expected</b>: False returned<br>
    * ************************************************
	 */

	t.day = 30;
	t.month = 13;
	t.year = 2020;
	t.hour = 12;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_NOK, time_set_utc(&t));

	/**
	 * <b>scenario</b>: Setting time with correct month.<br>
	 * <b>expected</b>: True returned<br>
    * ************************************************
	 */

	t.day = 30;
	t.month = 12;
	t.year = 2020;
	t.hour = 12;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_OK, time_set_utc(&t));

	/**
	 * <b>scenario</b>: Setting time with incorrect hour.<br>
	 * <b>expected</b>: False returned<br>
    * ************************************************
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 24;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_NOK, time_set_utc(&t));

	/**
	 * <b>scenario</b>: Setting time with correct hour.<br>
	 * <b>expected</b>: True returned<br>
    * ************************************************
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_OK, time_set_utc(&t));

	/**
	 * <b>scenario</b>: Setting time with incorrect minute.<br>
	 * <b>expected</b>: False returned<br>
    * ************************************************
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 60;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_NOK, time_set_utc(&t));

	/**
	 * <b>scenario</b>: Setting time with correct minute.<br>
	 * <b>expected</b>: True returned<br>
    * ************************************************
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 59;
	t.second = 30;
	t.msecond = 30;

	EXPECT_EQ(RETURN_OK, time_set_utc(&t));

	/**
	 * <b>scenario</b>: Setting time with incorrect second.<br>
	 * <b>expected</b>: False returned<br>
    * ************************************************
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 59;
	t.second = 60;
	t.msecond = 30;

	EXPECT_EQ(RETURN_NOK, time_set_utc(&t));

	/**
	 * <b>scenario</b>: Setting time with correct second.<br>
	 * <b>expected</b>: True returned<br>
    * ************************************************
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 59;
	t.second = 59;
	t.msecond = 30;

	EXPECT_EQ(RETURN_OK, time_set_utc(&t));

	/**
	 * <b>scenario</b>: Setting time with incorrect mseconds.<br>
	 * <b>expected</b>: False returned<br>
    * ************************************************
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 59;
	t.second = 59;
	t.msecond = 1000;

	EXPECT_EQ(RETURN_NOK, time_set_utc(&t));

	/**
	 * <b>scenario</b>: Setting time with incorrect mseconds.<br>
	 * <b>expected</b>: False returned<br>
    * ************************************************
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 59;
	t.second = 59;
	t.msecond = 54;

	EXPECT_EQ(RETURN_NOK, time_set_utc(&t));

	/**
	 * <b>scenario</b>: Setting time with correct mseconds.<br>
	 * <b>expected</b>: True returned<br>
    * ************************************************
	 */

	t.day = 30;
	t.month = 11;
	t.year = 2020;
	t.hour = 23;
	t.minute = 59;
	t.second = 59;
	t.msecond = 90;

	EXPECT_EQ(RETURN_OK, time_set_utc(&t));
}
/**
 * @test Setting summer/winter time
 */
TEST_F(timeFixture, time_set_utc)
{
	time_init();
	TimeItem t;
	EXPECT_EQ(RETURN_OK, time_register_callback(&fake_callback,TIME_PRIORITY_LOW));
	/**
	 * <b>scenario</b>: Setting time when winter time is active.<br>
	 * <b>expected</b>: Hour incremented by 1.<br>
    * ************************************************
	 */
	t.day = 20;
	t.month = 12;
	t.year = 2020;
	t.hour = 12;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	time_set_utc(&t);

	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](TimeItem* item)
			{
				if (!item) return;
				EXPECT_EQ(item->day, 20);
				EXPECT_EQ(item->month, 12);
				EXPECT_EQ(item->year, 2020);
				EXPECT_EQ(item->hour, 13);
				EXPECT_EQ(item->second, 30);
				EXPECT_EQ(item->msecond, 40);
			}));

	SysTick_Handler();
	time_watcher();


	/**
	 * <b>scenario</b>: Setting time when winter time is active and is about to midnight.<br>
	 * <b>expected</b>: Hour incremented by 1 and correctly switched to next day.<br>
    * ************************************************
	 */
	t.day = 20;
	t.month = 12;
	t.year = 2020;
	t.hour = 23;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	time_set_utc(&t);

	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](TimeItem* item)
			{
				if (!item) return;
				EXPECT_EQ(item->day, 21);
				EXPECT_EQ(item->month, 12);
				EXPECT_EQ(item->year, 2020);
				EXPECT_EQ(item->hour, 0);
				EXPECT_EQ(item->second, 30);
				EXPECT_EQ(item->msecond, 40);
			}));

	SysTick_Handler();
	time_watcher();

	/**
	 * <b>scenario</b>: Setting time when winter time is active and is about to midnight, last day of month.<br>
	 * <b>expected</b>: Hour incremented by 1 and correctly switched to next year.<br>
    * ************************************************
	 */
	t.day = 31;
	t.month = 12;
	t.year = 2020;
	t.hour = 23;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	time_set_utc(&t);

	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](TimeItem* item)
			{
				if (!item) return;
				EXPECT_EQ(item->day, 1);
				EXPECT_EQ(item->month, 1);
				EXPECT_EQ(item->year, 2021);
				EXPECT_EQ(item->hour, 0);
				EXPECT_EQ(item->second, 30);
				EXPECT_EQ(item->msecond, 40);
			}));

	SysTick_Handler();
	time_watcher();

	/**
	 * <b>scenario</b>: Setting time when summer time is active and is about to midnight, last day of month.<br>
	 * <b>expected</b>: Hour incremented by 2 and correctly switched to next year.<br>
    * ************************************************
	 */
	 time_set_winter_time(0);

	t.day = 31;
	t.month = 12;
	t.year = 2020;
	t.hour = 23;
	t.minute = 20;
	t.second = 30;
	t.msecond = 30;

	time_set_utc(&t);

	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](TimeItem* item)
			{
				if (!item) return;
				EXPECT_EQ(item->day, 1);
				EXPECT_EQ(item->month, 1);
				EXPECT_EQ(item->year, 2021);
				EXPECT_EQ(item->hour, 1);
				EXPECT_EQ(item->second, 30);
				EXPECT_EQ(item->msecond, 40);
			}));

	SysTick_Handler();
	time_watcher();

	EXPECT_EQ(RETURN_OK, time_unregister_callback(&fake_callback));
}
