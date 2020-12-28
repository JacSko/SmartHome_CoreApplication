#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/bathroom_fan.c"
#ifdef __cplusplus
}
#endif

#include "task_scheduler_mock.h"
#include "logger_mock.h"
#include "relays_board_mock.h"
#include "env_monitor_mock.h"

/* ============================= */
/**
 * @file bathroom_fan_tests.cpp
 *
 * @brief Unit tests of FAN module
 *
 * @details
 * This tests verifies behavior of FAN module
 *
 * @author Jacek Skowronek
 * @date 28/12/2020
 */
/* ============================= */


using namespace ::testing;

struct callbackMock
{
   MOCK_METHOD0(callback, void());
};

callbackMock* callMock;

void fake_callback()
{
   callMock->callback();
}

struct fanFixture : public ::testing::Test
{
   virtual void SetUp()
   {
      mock_sch_init();
      mock_logger_init();
      mock_rel_init();
      mock_env_init();
      callMock = new callbackMock;
   }

   virtual void TearDown()
   {
      mock_sch_deinit();
      mock_logger_deinit();
      mock_rel_deinit();
      mock_env_deinit();
      delete callMock;
   }
};

/**
 * @test
 */
TEST(envInitialization, env_initialization_tests)
{

}

/**
 * @test Reading sensors sequence test
 */
TEST_F(fanFixture, env_reading_test)
{

}
