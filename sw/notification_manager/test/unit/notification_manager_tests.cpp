#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/notification_manager.c"
#ifdef __cplusplus
}
#endif


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

/**
 * @test Initialization of LED module
 */
TEST(ledModuleInitialization, initialization)
{

}
