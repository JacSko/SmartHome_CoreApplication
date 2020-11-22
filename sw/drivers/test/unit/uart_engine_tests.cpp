#include "gtest/gtest.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/uart_engine.c"
#include "../../../../ext_lib/CMSIS/stubs/device/stm32f4xx.h"
#ifdef __cplusplus
}
#endif

#include "gpio_lib_mock.h"

/**
 * @brief Unit test of uartengine.
 *
 * All tests that verify behavior of uartengine module
 *
 * @file uart_engine_tests.cpp
 * @author  Jacek Skowronek
 * @date    01/11/2020
 */


using namespace ::testing;

struct callbackMock
{
	MOCK_METHOD1(callback, void(const char*));
};

callbackMock* callMock;

void fake_callback(const char* buf)
{
	callMock->callback(buf);
}

struct uartengineFixture : public ::testing::Test
{
	virtual void SetUp()
	{
		stm_stub_init();
		mock_gpio_init();
		callMock = new callbackMock();
	}

	virtual void TearDown()
	{
		stm_stub_deinit();
		mock_gpio_deinit();
		delete callMock;
	}
};

/**
 * @test Module initilization test
 */
TEST_F(uartengineFixture, engine_initialization)
{

	/**
	 * @<b>scenario<\b>: Module initialization.
	 * @<b>expected<\b>: RCC enabled, USART1 enabled, RXNEIE set, TXEIE not set.
	 */
	UART_Config cfg = {115200, '\n', 20, 15};

	EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(2);

	EXPECT_EQ(RETURN_OK, uartengine_initialize(&cfg));

	EXPECT_TRUE(READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN));
	EXPECT_TRUE(READ_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN));
	EXPECT_EQ(USART1->BRR, 100000000/cfg.baudrate);
	EXPECT_TRUE(READ_BIT(USART1->CR1, USART_CR1_RXNEIE));
	EXPECT_FALSE(READ_BIT(USART1->CR1, USART_CR1_TXEIE));
	EXPECT_TRUE(stm_stub_check_irq(USART1_IRQn, 1));
	uartengine_deinitialize();
}

/**
 * @test Sending string over UART
 */
TEST_F(uartengineFixture, string_send)
{

	char data[]= "STRING_TO_SEND\n";
	/**
	 * @<b>scenario<\b>: String send when UART not initialized.
	 * @<b>expected<\b>: Error returned.
	 */
	{
		EXPECT_EQ(RETURN_ERROR, uartengine_send_string(data));
	}

	/**
	 * @<b>scenario<\b>: String send.
	 * @<b>expected<\b>: Data written to buffer, TXEIE enabled.
	 */
	UART_Config cfg = {115200, '\n', 20, 15};
	EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(2);
	uartengine_initialize(&cfg);

	EXPECT_EQ(RETURN_OK, uartengine_send_string(data));
	EXPECT_EQ(tx_buf.head, 15);
	EXPECT_EQ(tx_buf.tail, 0);
	EXPECT_EQ(tx_buf.buf[0], 'S');
	EXPECT_EQ(tx_buf.buf[1], 'T');
	EXPECT_EQ(tx_buf.buf[2], 'R');
	EXPECT_EQ(tx_buf.buf[3], 'I');
	EXPECT_EQ(tx_buf.buf[4], 'N');
	EXPECT_EQ(tx_buf.buf[5], 'G');
	EXPECT_EQ(tx_buf.buf[6], '_');
	EXPECT_EQ(tx_buf.buf[7], 'T');
	EXPECT_EQ(tx_buf.buf[8], 'O');
	EXPECT_EQ(tx_buf.buf[9], '_');
	EXPECT_EQ(tx_buf.buf[10], 'S');
	EXPECT_EQ(tx_buf.buf[11], 'E');
	EXPECT_EQ(tx_buf.buf[12], 'N');
	EXPECT_EQ(tx_buf.buf[13], 'D');
	EXPECT_EQ(tx_buf.buf[14], '\n');
	EXPECT_TRUE(READ_BIT(USART1->CR1, USART_CR1_TXEIE));

	/* Simulate USART IRQ - read data from buffer */
	USART1->SR |=  USART_SR_TXE;
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'S');
	EXPECT_EQ(tx_buf.tail, 1);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'T');
	EXPECT_EQ(tx_buf.tail, 2);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'R');
	EXPECT_EQ(tx_buf.tail, 3);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'I');
	EXPECT_EQ(tx_buf.tail, 4);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'N');
	EXPECT_EQ(tx_buf.tail, 5);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'G');
	EXPECT_EQ(tx_buf.tail, 6);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, '_');
	EXPECT_EQ(tx_buf.tail, 7);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'T');
	EXPECT_EQ(tx_buf.tail, 8);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'O');
	EXPECT_EQ(tx_buf.tail, 9);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, '_');
	EXPECT_EQ(tx_buf.tail, 10);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'S');
	EXPECT_EQ(tx_buf.tail, 11);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'E');
	EXPECT_EQ(tx_buf.tail, 12);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'N');
	EXPECT_EQ(tx_buf.tail, 13);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'D');
	EXPECT_EQ(tx_buf.tail, 14);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, '\n');
	EXPECT_EQ(tx_buf.tail, 15);
	USART1_IRQHandler();
	EXPECT_EQ(tx_buf.tail, tx_buf.head);
	EXPECT_FALSE(READ_BIT(USART1->CR1, USART_CR1_TXEIE));

	/**
	 * @<b>scenario<\b>: String send when buffer overlap.
	 * @<b>expected<\b>: Data written to buffer, TXEIE enabled.
	 */
	char data2[]= "SECOND_STRING\n";
	EXPECT_EQ(RETURN_OK, uartengine_send_string(data2));
	EXPECT_EQ(tx_buf.head, 9);
	EXPECT_EQ(tx_buf.tail, 15);
	EXPECT_EQ(tx_buf.buf[15], 'S');
	EXPECT_EQ(tx_buf.buf[16], 'E');
	EXPECT_EQ(tx_buf.buf[17], 'C');
	EXPECT_EQ(tx_buf.buf[18], 'O');
	EXPECT_EQ(tx_buf.buf[19], 'N');
	EXPECT_EQ(tx_buf.buf[0], 'D');
	EXPECT_EQ(tx_buf.buf[1], '_');
	EXPECT_EQ(tx_buf.buf[2], 'S');
	EXPECT_EQ(tx_buf.buf[3], 'T');
	EXPECT_EQ(tx_buf.buf[4], 'R');
	EXPECT_EQ(tx_buf.buf[5], 'I');
	EXPECT_EQ(tx_buf.buf[6], 'N');
	EXPECT_EQ(tx_buf.buf[7], 'G');
	EXPECT_EQ(tx_buf.buf[8], '\n');
	EXPECT_TRUE(READ_BIT(USART1->CR1, USART_CR1_TXEIE));

	/* Simulate USART IRQ - read data from buffer */
	USART1->SR |=  USART_SR_TXE;
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'S');
	EXPECT_EQ(tx_buf.tail, 16);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'E');
	EXPECT_EQ(tx_buf.tail, 17);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'C');
	EXPECT_EQ(tx_buf.tail, 18);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'O');
	EXPECT_EQ(tx_buf.tail, 19);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'N');
	EXPECT_EQ(tx_buf.tail, 0);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'D');
	EXPECT_EQ(tx_buf.tail, 1);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, '_');
	EXPECT_EQ(tx_buf.tail, 2);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'S');
	EXPECT_EQ(tx_buf.tail, 3);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'T');
	EXPECT_EQ(tx_buf.tail, 4);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'R');
	EXPECT_EQ(tx_buf.tail, 5);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'I');
	EXPECT_EQ(tx_buf.tail, 6);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'N');
	EXPECT_EQ(tx_buf.tail, 7);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'G');
	EXPECT_EQ(tx_buf.tail, 8);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, '\n');
	EXPECT_EQ(tx_buf.tail, 9);
	USART1_IRQHandler();
	EXPECT_EQ(tx_buf.tail, tx_buf.head);
	EXPECT_FALSE(READ_BIT(USART1->CR1, USART_CR1_TXEIE));

	/**
	 * @<b>scenario<\b>: String send with CR LF ending.
	 * @<b>expected<\b>: Data written to buffer, TXEIE enabled.
	 */
	char data3[]= "AT_CMD\r\n";
	EXPECT_EQ(RETURN_OK, uartengine_send_string(data3));
	EXPECT_EQ(tx_buf.head, 17);
	EXPECT_EQ(tx_buf.tail, 9);
	EXPECT_EQ(tx_buf.buf[9], 'A');
	EXPECT_EQ(tx_buf.buf[10], 'T');
	EXPECT_EQ(tx_buf.buf[11], '_');
	EXPECT_EQ(tx_buf.buf[12], 'C');
	EXPECT_EQ(tx_buf.buf[13], 'M');
	EXPECT_EQ(tx_buf.buf[14], 'D');
	EXPECT_EQ(tx_buf.buf[15], '\r');
	EXPECT_EQ(tx_buf.buf[16], '\n');
	EXPECT_TRUE(READ_BIT(USART1->CR1, USART_CR1_TXEIE));

	/* Simulate USART IRQ - read data from buffer */
	USART1->SR |=  USART_SR_TXE;
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'A');
	EXPECT_EQ(tx_buf.tail, 10);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'T');
	EXPECT_EQ(tx_buf.tail, 11);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, '_');
	EXPECT_EQ(tx_buf.tail, 12);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'C');
	EXPECT_EQ(tx_buf.tail, 13);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'M');
	EXPECT_EQ(tx_buf.tail, 14);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, 'D');
	EXPECT_EQ(tx_buf.tail, 15);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, '\r');
	EXPECT_EQ(tx_buf.tail, 16);
	USART1_IRQHandler();
	EXPECT_EQ(USART1->DR, '\n');
	EXPECT_EQ(tx_buf.tail, 17);
	USART1_IRQHandler();

	EXPECT_EQ(tx_buf.tail, tx_buf.head);
	EXPECT_FALSE(READ_BIT(USART1->CR1, USART_CR1_TXEIE));

	uartengine_deinitialize();
}

/**
 * @test Reading string from UART
 */
TEST_F(uartengineFixture, string_read)
{
	/**
	 * @<b>scenario<\b>: String read.
	 * @<b>expected<\b>: Data written to provided buffer, NULL terminated.
	 */
	UART_Config cfg = {115200, '\n', 20, 15};
	EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(2);
	uartengine_initialize(&cfg);
	uartengine_register_callback(&fake_callback);

	USART1->SR |= USART_SR_RXNE;

	USART1->DR = 'R';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 1);
	USART1->DR = 'E';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 2);
	USART1->DR = 'A';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 3);
	USART1->DR = 'D';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 4);
	USART1->DR = '_';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 5);
	USART1->DR = 'S';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 6);
	USART1->DR = 'T';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 7);
	USART1->DR = 'R';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 8);
	USART1->DR = 'I';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 9);
	USART1->DR = 'N';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 10);
	USART1->DR = 'G';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 11);
	USART1->DR = '\n';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 12);

	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](const char* buf)
			{
				EXPECT_EQ(buf[0], 'R');
				EXPECT_EQ(buf[1], 'E');
				EXPECT_EQ(buf[2], 'A');
				EXPECT_EQ(buf[3], 'D');
				EXPECT_EQ(buf[4], '_');
				EXPECT_EQ(buf[5], 'S');
				EXPECT_EQ(buf[6], 'T');
				EXPECT_EQ(buf[7], 'R');
				EXPECT_EQ(buf[8], 'I');
				EXPECT_EQ(buf[9], 'N');
				EXPECT_EQ(buf[10], 'G');
				EXPECT_EQ(buf[11], '\0');
			}));
	uartengine_string_watcher();
	EXPECT_EQ(rx_buf.head, rx_buf.tail);

	USART1->DR = 'S';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 13);
	USART1->DR = 'E';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 14);
	USART1->DR = 'C';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 15);
	USART1->DR = 'O';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 16);
	USART1->DR = 'N';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 17);
	USART1->DR = 'D';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 18);
	USART1->DR = '_';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 19);
	USART1->DR = 'R';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 0);
	USART1->DR = 'E';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 1);
	USART1->DR = 'A';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 2);
	USART1->DR = 'D';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 3);
	USART1->DR = '\n';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 4);

	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](const char* buf)
			{
				EXPECT_EQ(buf[0], 'S');
				EXPECT_EQ(buf[1], 'E');
				EXPECT_EQ(buf[2], 'C');
				EXPECT_EQ(buf[3], 'O');
				EXPECT_EQ(buf[4], 'N');
				EXPECT_EQ(buf[5], 'D');
				EXPECT_EQ(buf[6], '_');
				EXPECT_EQ(buf[7], 'R');
				EXPECT_EQ(buf[8], 'E');
				EXPECT_EQ(buf[9], 'A');
				EXPECT_EQ(buf[10], 'D');
				EXPECT_EQ(buf[11], '\0');
			}));
	uartengine_string_watcher();
	EXPECT_EQ(rx_buf.head, rx_buf.tail);


	USART1->DR = 'A';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 5);
	USART1->DR = 'T';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 6);
	USART1->DR = '_';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 7);
	USART1->DR = 'R';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 8);
	USART1->DR = 'E';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 9);
	USART1->DR = 'P';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 10);
	USART1->DR = 'L';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 11);
	USART1->DR = 'Y';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 12);
	USART1->DR = '\r';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 12);
	USART1->DR = '\n';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 13);

	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](const char* buf)
			{
				EXPECT_EQ(buf[0], 'A');
				EXPECT_EQ(buf[1], 'T');
				EXPECT_EQ(buf[2], '_');
				EXPECT_EQ(buf[3], 'R');
				EXPECT_EQ(buf[4], 'E');
				EXPECT_EQ(buf[5], 'P');
				EXPECT_EQ(buf[6], 'L');
				EXPECT_EQ(buf[7], 'Y');
				EXPECT_EQ(buf[11], '\0');
			}));
	uartengine_string_watcher();
	EXPECT_EQ(rx_buf.head, rx_buf.tail);

	uartengine_deinitialize();
}

/**
 * @test Adding/removing callbacks tests
 */
TEST_F(uartengineFixture, callback_add_remove)
{

	for (uint8_t i = 0; i < UART_ENGINE_CALLBACK_SIZE; i++)
	{
		CALLBACKS[i] = NULL;
	}
	/**
	 * @<b>scenario<\b>: Callback list empty - adding one callback.
	 * @<b>expected<\b>: Callback placed on first place.
	 */
	EXPECT_EQ(RETURN_OK, uartengine_register_callback(&fake_callback));
	EXPECT_CALL(*callMock, callback(_));
	uartengine_notify_callbacks();

	/**
	 * @<b>scenario<\b>: Callback list full, calling callbacks.
	 * @<b>expected<\b>: All callbacks called.
	 */
	for (uint8_t i = 1; i < UART_ENGINE_CALLBACK_SIZE; i++)
	{
		EXPECT_EQ(RETURN_OK, uartengine_register_callback(&fake_callback));
	}
	EXPECT_CALL(*callMock, callback(_)).Times(5);
	uartengine_notify_callbacks();
	/**
	 * @<b>scenario<\b>: Callback list full, adding new callback.
	 * @<b>expected<\b>: New callback not added.
	 */
	EXPECT_EQ(RETURN_ERROR, uartengine_register_callback(&fake_callback));
	EXPECT_CALL(*callMock, callback(_)).Times(5);
	uartengine_notify_callbacks();

	/**
	 * @<b>scenario<\b>: Callback list full, unregister callback.
	 * @<b>expected<\b>: Callback unregistered
	 */
	EXPECT_EQ(RETURN_OK, uartengine_unregister_callback(&fake_callback));
	EXPECT_CALL(*callMock, callback(_)).Times(4);
	uartengine_notify_callbacks();

	/**
	 * @<b>scenario<\b>: Callback list not full, unregister 2 more callbacks.
	 * @<b>expected<\b>: Callback unregistered
	 */
	EXPECT_EQ(RETURN_OK, uartengine_unregister_callback(&fake_callback));
	EXPECT_EQ(RETURN_OK, uartengine_unregister_callback(&fake_callback));
	EXPECT_CALL(*callMock, callback(_)).Times(2);
	uartengine_notify_callbacks();

	/**
	 * @<b>scenario<\b>: Callback list not contiguous, adding new callback.
	 * @<b>expected<\b>: Callback added
	 */
	EXPECT_EQ(RETURN_OK, uartengine_register_callback(&fake_callback));
	EXPECT_CALL(*callMock, callback(_)).Times(3);
	uartengine_notify_callbacks();

	for (uint8_t i = 0; i < UART_ENGINE_CALLBACK_SIZE; i++)
	{
		CALLBACKS[i] = NULL;
	}
}

/**
 * @test Reading string from UART when unexpected line endings (CR, LF) occurs.
 */
TEST_F(uartengineFixture, string_read_special_data)
{
	/**
	 * @<b>scenario<\b>: CRLF sequence received on begin and end of the string.
	 * @<b>expected<\b>: CRLF sequences shall be filtered out.
	 */
	UART_Config cfg = {115200, '\n', 20, 15};
	EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(2);
	uartengine_initialize(&cfg);
	uartengine_register_callback(&fake_callback);

	USART1->SR |= USART_SR_RXNE;

	USART1->DR = '\r';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 0);
	USART1->DR = '\n';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 0);
	USART1->DR = 'O';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 1);
	USART1->DR = 'K';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 2);
	USART1->DR = '\r';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 2);
	USART1->DR = '\n';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 3);
	EXPECT_EQ(rx_buf.tail, 0);
	EXPECT_EQ(rx_buf.string_cnt, 1);

	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](const char* buf)
			{
				EXPECT_EQ(buf[0], 'O');
				EXPECT_EQ(buf[1], 'K');
				EXPECT_EQ(buf[2], '\0');
			}));

	uartengine_string_watcher(); //shouldn't call callback, because first string is empty
	uartengine_string_watcher();
	EXPECT_EQ(rx_buf.head, rx_buf.tail);
	EXPECT_EQ(rx_buf.string_cnt, 0);

	USART1->DR = '\n';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 3);
	USART1->DR = '\r';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 3);
	USART1->DR = 'O';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 4);
	USART1->DR = 'K';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 5);
	USART1->DR = '\n';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 6);
	USART1->DR = '\r';
	USART1_IRQHandler();
	EXPECT_EQ(rx_buf.head, 6);
	EXPECT_EQ(rx_buf.tail, 3);
	EXPECT_EQ(rx_buf.string_cnt, 1);

	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](const char* buf)
			{
				EXPECT_EQ(buf[0], 'O');
				EXPECT_EQ(buf[1], 'K');
				EXPECT_EQ(buf[2], '\0');
			}));

	uartengine_string_watcher(); //shouldn't call callback, because first string is empty
	uartengine_string_watcher();
	EXPECT_EQ(rx_buf.head, rx_buf.tail);
	EXPECT_EQ(rx_buf.string_cnt, 0);

	uartengine_deinitialize();

}
