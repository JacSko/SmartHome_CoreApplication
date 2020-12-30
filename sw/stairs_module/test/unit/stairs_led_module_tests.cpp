#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/stairs_led_module.c"
#ifdef __cplusplus
}
#endif

/* ============================= */
/**
 * @file stairs_led_module_tests.cpp
 *
 * @brief Unit tests of Stairs LED module
 *
 * @details
 * This tests verifies behavior of Stairs LED module
 *
 * @author Jacek Skowronek
 * @date 31/12/2020
 */
/* ============================= */


using namespace ::testing;


struct ledFixture : public ::testing::Test
{
	virtual void SetUp()
	{
	}

	virtual void TearDown()
	{
	}
};

/**
 * @test Sending log over uart
 */
TEST_F(ledFixture, send_log)
{
	/**
	 * <b>scenario</b>: Send debug string.<br>
	 * <b>expected</b>: Callback called.<br>
    * ************************************************
	 */
}
