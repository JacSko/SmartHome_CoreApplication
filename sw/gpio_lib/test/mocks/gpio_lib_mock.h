#ifndef _GPIO_LIB_MOCK_
#define _GPIO_LIB_MOCK_

#include "gpio_lib.h"
#include "gmock/gmock.h"

struct gpioCfgMock
{
	MOCK_METHOD3(gpio_pin_cfg, void(GPIO_TypeDef * const __restrict__, GpioPin_t, GpioMode_t));
};


gpioCfgMock* gpio_lib_mock;

void mock_gpio_init()
{
	gpio_lib_mock = new (gpioCfgMock);
}

void mock_gpio_deinit()
{
	delete gpio_lib_mock;
}

void gpio_pin_cfg(GPIO_TypeDef * const __restrict__ port, GpioPin_t pin, GpioMode_t mode)
{
	gpio_lib_mock->gpio_pin_cfg(port, pin, mode);
}

#endif
