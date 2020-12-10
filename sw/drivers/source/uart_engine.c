#include <stdlib.h>
#include <string.h>

#include "stm32f4xx.h"
#include "core_cmFunc.h"
#include "uart_engine.h"
#include "gpio_lib.h"
#include "return_codes.h"
/*
 * UART engine summary:
 * - UART2 peripherial
 * - UART_TX = PA2
 * - UART_RX = PA3
 */

/* INTERNAL functions declaration */
void start_sending();
void stop_sending();
void uartengine_notify_callbacks();
#define UART_ENGINE_CALLBACK_SIZE 5
RET_CODE uartengine_get_string_from_buffer();

typedef struct
{
	char * buf;
	uint16_t tail;
	uint16_t head;
	uint8_t string_cnt;
	uint16_t bytes_cnt;
} UART_BUFFER;

UARTEngine_Config uart_config;
volatile UART_BUFFER uart_tx_buf;
volatile UART_BUFFER uart_rx_buf;
char* rx_string;

void (*UART_CALLBACKS[UART_ENGINE_CALLBACK_SIZE])(const char *);


RET_CODE uartengine_initialize(const UARTEngine_Config* cfg)
{
	RET_CODE result = RETURN_OK;
	uart_config = *cfg;

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	__DSB();

	USART2->CR1 |= USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_TE;
	USART2->BRR = (50000000/uart_config.baudrate);
	gpio_pin_cfg(GPIOA, PA3, gpio_mode_AF7_OD_PU_LS);
	gpio_pin_cfg(GPIOA, PA2, gpio_mode_AF7_PP_LS);

	uart_tx_buf.buf = (char*) malloc (sizeof(char)*uart_config.buffer_size);
	uart_rx_buf.buf = (char*) malloc (sizeof(char)*uart_config.buffer_size);
	rx_string = (char*) malloc (sizeof(char)*uart_config.string_size);

	uart_tx_buf.head = 0;
	uart_tx_buf.tail = 0;
	uart_tx_buf.string_cnt = 0; /* not used */
	uart_tx_buf.bytes_cnt = 0;  /* not user */

	uart_rx_buf.head = 0;
	uart_rx_buf.tail = 0;
	uart_rx_buf.string_cnt = 0;
	uart_rx_buf.bytes_cnt = 0;

	if (!uart_rx_buf.buf || !uart_rx_buf.buf || !rx_string)
	{
		result = RETURN_ERROR;
	}

	NVIC_EnableIRQ(USART2_IRQn);

	return result;
}

void uartengine_deinitialize()
{
	free(uart_tx_buf.buf);
	free(uart_rx_buf.buf);
	free(rx_string);
	uart_tx_buf.buf = NULL;
	uart_rx_buf.buf = NULL;
	rx_string = NULL;
	for (uint8_t i = 0; i < UART_ENGINE_CALLBACK_SIZE; i++)
	{
		UART_CALLBACKS[i] = NULL;
	}

}

RET_CODE uartengine_send_string(const char * buffer)
{
	if (!uart_tx_buf.buf)
	{
		return RETURN_ERROR;
	}

	char c;
	__disable_irq();
	while (*buffer)
	{
		c = *buffer;
		uart_tx_buf.buf[uart_tx_buf.head] = c;
		uart_tx_buf.head++;
		if (uart_tx_buf.head == uart_config.buffer_size)
		{
			uart_tx_buf.head = 0;
		}
		buffer++;
	}
	__enable_irq();
	start_sending();
	return RETURN_OK;
}

RET_CODE uartengine_can_read_string()
{
	RET_CODE result = RETURN_NOK;

	if (uart_rx_buf.string_cnt > 0)
	{
		uartengine_get_string_from_buffer();
		if (strlen(rx_string) > 0)
		{
			result = RETURN_OK;
		}
	}
	return result;
}

const char* uartengine_get_string()
{
	return rx_string;
}
void uartengine_clear_rx()
{
	uart_rx_buf.tail = uart_rx_buf.head;
	uart_rx_buf.string_cnt = 0;
	uart_rx_buf.bytes_cnt = 0;
}

RET_CODE uartengine_get_string_from_buffer()
{
	char* buffer = rx_string;
	if (!uart_rx_buf.buf || uart_rx_buf.string_cnt == 0 || !buffer)
	{
		return RETURN_ERROR;
	}

	char c;
	while (1)
	{
		c = uart_rx_buf.buf[uart_rx_buf.tail];
		uart_rx_buf.tail++;
		uart_rx_buf.bytes_cnt--;
		if (uart_rx_buf.tail == uart_config.buffer_size)
		{
			uart_rx_buf.tail = 0;
		}
		if (c != '\r' && c != '\n')
		{
			*buffer = c;
		}
		else
		{
			*buffer = 0x00;
		}
		if (c == '\n')
		{
			*buffer = 0x00;
			break;
		}
		buffer++;

	}
	uart_rx_buf.string_cnt--;
	return RETURN_OK;
}

uint16_t uartengine_count_bytes()
{
	return uart_rx_buf.bytes_cnt;
}

const uint8_t* uartengine_get_bytes()
{
	char* buffer = rx_string;
	if (!uart_rx_buf.buf || uart_rx_buf.bytes_cnt == 0 || !buffer)
	{
		return NULL;
	}

	while (uart_rx_buf.bytes_cnt)
	{
		char c = uart_rx_buf.buf[uart_rx_buf.tail];
		*buffer = c;
		if (c == '\n')
		{
			uart_rx_buf.string_cnt--;
		}
		uart_rx_buf.tail++;
		uart_rx_buf.bytes_cnt--;
		if (uart_rx_buf.tail == uart_config.buffer_size)
		{
			uart_rx_buf.tail = 0;
		}
		buffer++;

	}
	return (uint8_t*)rx_string;
}

RET_CODE uartengine_register_callback(void(*callback)(const char *))
{
	RET_CODE result = RETURN_ERROR;
	for (uint8_t i = 0; i < UART_ENGINE_CALLBACK_SIZE; i++)
	{
		if (UART_CALLBACKS[i] == NULL)
		{
			UART_CALLBACKS[i] = callback;
			result = RETURN_OK;
			break;
		}
	}
	return result;
}

RET_CODE uartengine_unregister_callback(void(*callback)(const char *))
{
	RET_CODE result = RETURN_ERROR;
	for (uint8_t i = 0; i < UART_ENGINE_CALLBACK_SIZE; i++)
	{
		if (UART_CALLBACKS[i] == callback)
		{
			UART_CALLBACKS[i] = NULL;
			result = RETURN_OK;
			break;
		}
	}
	return result;
}

void uartengine_notify_callbacks()
{
	for (uint8_t i = 0; i < UART_ENGINE_CALLBACK_SIZE; i++)
	{
		if (UART_CALLBACKS[i] != NULL)
		{
			UART_CALLBACKS[i](rx_string);
		}
	}
}

void uartengine_string_watcher()
{
	if (uartengine_can_read_string())
	{
		uartengine_notify_callbacks();
	}
}

void start_sending()
{
	USART2->CR1 |= USART_CR1_TXEIE;
}
void stop_sending()
{
	USART2->CR1 &= ~USART_CR1_TXEIE;
}

void USART2_IRQHandler (void)
{
	if (USART2->SR & USART_SR_RXNE){
		char c = USART2->DR;
		uart_rx_buf.buf[uart_rx_buf.head] = USART2->DR;
		uart_rx_buf.head++;
		uart_rx_buf.bytes_cnt++;
		if (c == '\n')
		{
			uart_rx_buf.string_cnt++;
		}

		if (uart_rx_buf.head == uart_config.buffer_size)
		{
			uart_rx_buf.head = 0;
		}

	}

	if (USART2->SR & USART_SR_TXE){
		if (uart_tx_buf.head == uart_tx_buf.tail)
		{
			stop_sending();
		}
		else
		{
			USART2->DR = uart_tx_buf.buf[uart_tx_buf.tail];
			uart_tx_buf.tail++;
			if (uart_tx_buf.tail == uart_config.buffer_size)
			{
				uart_tx_buf.tail = 0;
			}
		}
	}
}
