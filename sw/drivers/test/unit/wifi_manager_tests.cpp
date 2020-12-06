#include "gtest/gtest.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/wifi_manager.c"
#ifdef __cplusplus
}
#endif

#include "wifidriver_mock.h"
#include "time_counter_mock.h"
#include "logger_mock.h"
/**
 * @brief Unit test of wifi manager.
 *
 * All tests that verify behavior of WiFi manager module
 *
 * @file wifi_manager_tests.cpp
 * @author  Jacek Skowronek
 * @date    06/12/2020
 */


using namespace ::testing;

struct callbackMock
{
	MOCK_METHOD2(callback, void(ServerClientID, const char*));
};

callbackMock* callMock;

void fake_callback(ServerClientID id, const char* data)
{
	callMock->callback(id, data);
}

struct wifiMgrFixture : public ::testing::Test
{
	virtual void SetUp()
	{
		mock_logger_init();
		mock_wifidriver_init();
		callMock = new callbackMock();
	}

	virtual void TearDown()
	{
		mock_logger_deinit();
		mock_wifidriver_init();
		delete callMock;
	}
};

/**
 * @test WiFi manager initialization
 */
TEST_F(wifiMgrFixture, manager_initialization)
{
	/**
	 * @<b>scenario<\b>: Module initialization - cannot initialize wifi driver.
	 * @<b>expected<\b>: RETURN_NOK returned.
	 */
}
