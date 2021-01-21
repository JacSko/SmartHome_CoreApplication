#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/system_timestamp.c"
#include "../../../../ext_lib/CMSIS/stubs/device/stm32f4xx.h"
#ifdef __cplusplus
}
#endif

/* ============================= */
/**
 * @file system_timestamp_tests.cpp
 *
 * @brief Unit tests of timestamp module
 *
 * @details
 * This tests verifies behavior of timestamp module
 *
 * @author Jacek Skowronek
 * @date 21/01/2021
 */
/* ============================= */

using namespace ::testing;

struct tsFixture : public ::testing::Test
{
   virtual void SetUp()
   {
      stm_stub_init();
   }

   virtual void TearDown()
   {
      stm_stub_deinit();
   }
};

/**
 * @test Module initialization
 */
TEST_F(tsFixture, ts_init_tests)
{
   /**
    * <b>scenario</b>: Module initialization.<br>
    * <b>expected</b>: Timer started and is counting.<br>
    * ************************************************
    */
   ts_init();
   EXPECT_TRUE(RCC->APB1ENR & RCC_APB1ENR_TIM4EN);
   EXPECT_EQ(TIM4->CNT, 0);
   EXPECT_TRUE(TIM4->CR1 & TIM_CR1_CEN);

}

/**
 * @test Getting diff tests
 */
TEST_F(tsFixture, ts_diff_tests)
{

   /* One CNT means 0,5ms */

   /**
    * <b>scenario</b>: Getting diff without variable overlap.<br>
    * <b>expected</b>: Correct diff returned.<br>
    * ************************************************
    */
   ts_init();
   system_timestamp = 10;
   uint32_t timestamp = ts_get();

   system_timestamp = 40;
   EXPECT_EQ(ts_get_diff(timestamp), 3000);

   system_timestamp = 60;
   EXPECT_EQ(ts_get_diff(timestamp), 5000);

   /**
    * <b>scenario</b>: Getting diff with variable overlap.<br>
    * <b>expected</b>: Correct diff returned.<br>
    * ************************************************
    */
   ts_init();
   system_timestamp = UINT16_MAX - 10;
   timestamp = ts_get();

   system_timestamp = 20;
   EXPECT_EQ(ts_get_diff(timestamp), 3000);


}
