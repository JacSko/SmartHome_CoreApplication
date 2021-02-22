#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/socket_driver.c"
#ifdef __cplusplus
}
#endif

#include "logger_mock.h"

/* ============================= */
/**
 * @file notification_manager_tests.cpp
 *
 * @brief Unit tests of Notification manager module
 *
 * @details
 * This tests verifies behavior of Notification manager module
 *
 * @author Jacek Skowronek
 * @date 23/01/2021
 */
/* ============================= */


using namespace ::testing;

struct ntfmgrFixture : public ::testing::Test
{
   virtual void SetUp()
   {

   }

   virtual void TearDown()
   {

   }
};

TEST(socketDriverTests, initialization)
{
}
