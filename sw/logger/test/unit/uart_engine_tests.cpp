#include "gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/uart_engine.c"
#include "../../../../ext_lib/CMSIS/stubs/device/stm32f4xx.h"
#ifdef __cplusplus
}
#endif

struct uartengineFixture : public ::testing::Test
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

TEST_F(uartengineFixture, engine_initialization)
{
	UART_Config cfg = {115200, '\n', 20};

	EXPECT_EQ(RETURN_OK, uartengine_initialize(&cfg));

	EXPECT_TRUE(READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN));
	EXPECT_TRUE(READ_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN));

	uartengine_deinitialize();
}
