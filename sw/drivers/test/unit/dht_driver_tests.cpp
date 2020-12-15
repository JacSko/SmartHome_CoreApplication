#include "gtest/gtest.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/dht_driver.c"
#include "../../../../ext_lib/CMSIS/stubs/device/stm32f4xx.h"
#ifdef __cplusplus
}
#endif

#include "gpio_lib_mock.h"
#include "task_scheduler_mock.h"

/* ============================= */
/**
 * @file dht_driver_tests.cpp
 *
 * @brief Unit tests of DHTXX Driver module
 *
 * @details
 * This tests verifies behavior of DHTXX Driver module
 *
 * @author Jacek Skowronek
 * @date 15/12/2020
 */
/* ============================= */


using namespace ::testing;

struct callbackMock
{
	MOCK_METHOD1(callback, void(const char*));
};

callbackMock* callMock;

void fake_callback(const char* buf)
{
	callMock->callback(buf);
}

struct dhtDriverFixture : public ::testing::Test
{
	virtual void SetUp()
	{
		stm_stub_init();
		mock_gpio_init();
		mock_sch_init();
		callMock = new callbackMock();
	}

	virtual void TearDown()
	{
		stm_stub_deinit();
		mock_gpio_deinit();
		mock_sch_deinit();
		delete callMock;
	}
};

/**
 * @test Module initilization test
 */
TEST_F(dhtDriverFixture, engine_initialization)
{

	/**
	 * <b>scenario</b>: <br>
	 * <b>expected</b>: <br>
	 * ************************************************
	 */

}
