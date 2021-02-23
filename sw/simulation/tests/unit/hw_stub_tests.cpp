#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/hw_stub.c"
#ifdef __cplusplus
}
#endif

#include "socket_driver_mock.h"
#include "logger_mock.h"

/* ============================= */
/**
 * @file hw_stub_tests.cpp
 *
 * @brief Unit tests of STM HW stub module
 *
 * @author Jacek Skowronek
 * @date 23/02/2021
 */
/* ============================= */


using namespace ::testing;


struct hwstubFixture : public ::testing::Test
{
   virtual void SetUp()
   {
   }

   virtual void TearDown()
   {
   }
};

/**
 * @test
 */
TEST(hwstubInitialization, env_initialization_tests)
{
   /**
    * <b>scenario</b>: ENV initialize - no config provided.<br>
    * <b>expected</b>: Module not intialized.<br>
    * ************************************************
    */
}
