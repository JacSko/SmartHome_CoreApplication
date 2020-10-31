#include "gtest/gtest.h"
#include "stm32_stub.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../source/test.c"
#ifdef __cplusplus
}
#endif

TEST(test1, test1)
{
	stm_stub_init();
	add2(1,1);
	EXPECT_EQ(GPIOC->IDR, 0xFF);
	stm_stub_deinit();
}

