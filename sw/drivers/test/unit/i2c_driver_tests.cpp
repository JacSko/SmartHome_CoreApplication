#include "gtest/gtest.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/i2c_driver.c"
#include "../../../../ext_lib/CMSIS/stubs/device/stm32f4xx.h"
#ifdef __cplusplus
}
#endif

#include "logger_mock.h"
#include "gpio_lib_mock.h"
#include "task_scheduler_mock.h"

/* ============================= */
/**
 * @file i2c_driver_tests.cpp
 *
 * @brief Unit tests of I2C Driver module
 *
 * @details
 * This tests verifies behavior of I2C Driver module
 *
 * @author Jacek Skowronek
 * @date 18/12/2020
 */
/* ============================= */


using namespace ::testing;

struct callbackMock
{
};

callbackMock* callMock;

void fake_callback()
{
}

struct i2cDriverFixture : public ::testing::Test
{
   virtual void SetUp()
   {
      mock_logger_init();
      stm_stub_init();
      mock_gpio_init();
      mock_sch_init();
      callMock = new callbackMock();
   }

   virtual void TearDown()
   {
      mock_logger_deinit();
      stm_stub_deinit();
      mock_gpio_deinit();
      mock_sch_deinit();
      delete callMock;
   }
};

/**
 * @test
 */
TEST_F(i2cDriverFixture, reading_one_sensor_test)
{

   /**
    * <b>scenario</b>:  <br>
    * <b>expected</b>:  <br>
    * ************************************************
    */
   i2c_initialize();
}
