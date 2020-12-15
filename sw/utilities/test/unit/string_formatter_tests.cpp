#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/string_formatter.c"
#ifdef __cplusplus
}
#endif

/* ============================= */
/**
 * @file string_formatter_tests.cpp
 *
 * @brief Unit tests of String Formatter utility
 *
 * @details
 * This tests verifies behavior of String Formatter utility
 *
 * @author Jacek Skowronek
 * @date 15/12/2020
 */
/* ============================= */


using namespace ::testing;

struct stringFormatterFixture : public ::testing::Test
{
   const uint16_t  LOGGER_BUFFER_SIZE = 512;
   virtual void SetUp()
   {
   }

   virtual void TearDown()
   {
   }
};

/**
 * @test String formatting tests
 */
TEST_F(stringFormatterFixture, string_formatting_tests)
{

   char result [50];
   /**
    * <b>scenario</b>: Formatting string according to pattern.<br>
    * <b>expected</b>: String formatted as expected.<br>
    * ************************************************
    */
   string_format(result, "RESULT: %d %x %u %c\n", -10, 11, 12, 'x');
   EXPECT_STREQ(result, "RESULT: -10 B 12 x\n");

   /**
    * <b>scenario</b>: Formatting string according to pattern with precision.<br>
    * <b>expected</b>: String formatted as expected.<br>
    * ************************************************
    */
   string_format(result, "RESULT: %d %x %.4u %s\n", -10, 11, 12, "STR");
   EXPECT_STREQ(result, "RESULT: -10 B 0012 STR\n");
}
