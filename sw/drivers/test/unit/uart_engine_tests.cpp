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

/* ============================= */
/**
 * @file uart_engine_tests.cpp
 *
 * @brief Unit tests of UART engine module
 *
 * @details
 * This tests verifies behavior of UART engine module
 *
 * @author Jacek Skowronek
 * @date 01/11/2020
 */
/* ============================= */

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
	 * <b>scenario</b>: Module initialization.<br>
	 * <b>expected</b>: RCC enabled, USART2 enabled, RXNEIE set, TXEIE not set.<br>
    * ************************************************
	 */
	UARTEngine_Config cfg = {115200, 20, 15};

	EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(2);

	EXPECT_EQ(RETURN_OK, uartengine_initialize(&cfg));

	EXPECT_TRUE(READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN));
	EXPECT_TRUE(READ_BIT(RCC->APB1ENR, RCC_APB1ENR_USART2EN));
	EXPECT_EQ(USART2->BRR, 50000000/cfg.baudrate);
	EXPECT_TRUE(READ_BIT(USART2->CR1, USART_CR1_RXNEIE));
	EXPECT_FALSE(READ_BIT(USART2->CR1, USART_CR1_TXEIE));
	EXPECT_TRUE(stm_stub_check_irq(USART2_IRQn, 1));
	uartengine_deinitialize();
}

/**
 * @test Sending string over UART
 */
TEST_F(uartengineFixture, string_send)
{

	char data[]= "STRING_TO_SEND\n";
	/**
	 * <b>scenario</b>: String send when UART not initialized.<br>
	 * <b>expected</b>: Error returned.<br>
    * ************************************************
	 */
	{
		EXPECT_EQ(RETURN_ERROR, uartengine_send_string(data));
	}

	/**
	 * <b>scenario</b>: String send.<br>
	 * <b>expected</b>: Data written to buffer, TXEIE enabled.<br>
    * ************************************************
	 */
	UARTEngine_Config cfg = {115200, 20, 15};
	EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(2);
	uartengine_initialize(&cfg);

	EXPECT_EQ(RETURN_OK, uartengine_send_string(data));
	EXPECT_EQ(uart_tx_buf.head, 15);
	EXPECT_EQ(uart_tx_buf.tail, 0);
	EXPECT_EQ(uart_tx_buf.buf[0], 'S');
	EXPECT_EQ(uart_tx_buf.buf[1], 'T');
	EXPECT_EQ(uart_tx_buf.buf[2], 'R');
	EXPECT_EQ(uart_tx_buf.buf[3], 'I');
	EXPECT_EQ(uart_tx_buf.buf[4], 'N');
	EXPECT_EQ(uart_tx_buf.buf[5], 'G');
	EXPECT_EQ(uart_tx_buf.buf[6], '_');
	EXPECT_EQ(uart_tx_buf.buf[7], 'T');
	EXPECT_EQ(uart_tx_buf.buf[8], 'O');
	EXPECT_EQ(uart_tx_buf.buf[9], '_');
	EXPECT_EQ(uart_tx_buf.buf[10], 'S');
	EXPECT_EQ(uart_tx_buf.buf[11], 'E');
	EXPECT_EQ(uart_tx_buf.buf[12], 'N');
	EXPECT_EQ(uart_tx_buf.buf[13], 'D');
	EXPECT_EQ(uart_tx_buf.buf[14], '\n');
	EXPECT_TRUE(READ_BIT(USART2->CR1, USART_CR1_TXEIE));

	/* Simulate USART IRQ - read data from buffer */
	USART2->SR |=  USART_SR_TXE;
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'S');
	EXPECT_EQ(uart_tx_buf.tail, 1);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'T');
	EXPECT_EQ(uart_tx_buf.tail, 2);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'R');
	EXPECT_EQ(uart_tx_buf.tail, 3);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'I');
	EXPECT_EQ(uart_tx_buf.tail, 4);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'N');
	EXPECT_EQ(uart_tx_buf.tail, 5);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'G');
	EXPECT_EQ(uart_tx_buf.tail, 6);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, '_');
	EXPECT_EQ(uart_tx_buf.tail, 7);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'T');
	EXPECT_EQ(uart_tx_buf.tail, 8);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'O');
	EXPECT_EQ(uart_tx_buf.tail, 9);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, '_');
	EXPECT_EQ(uart_tx_buf.tail, 10);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'S');
	EXPECT_EQ(uart_tx_buf.tail, 11);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'E');
	EXPECT_EQ(uart_tx_buf.tail, 12);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'N');
	EXPECT_EQ(uart_tx_buf.tail, 13);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'D');
	EXPECT_EQ(uart_tx_buf.tail, 14);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, '\n');
	EXPECT_EQ(uart_tx_buf.tail, 15);
	USART2_IRQHandler();
	EXPECT_EQ(uart_tx_buf.tail, uart_tx_buf.head);
	EXPECT_FALSE(READ_BIT(USART2->CR1, USART_CR1_TXEIE));

	/**
	 * <b>scenario</b>: String send when buffer overlap.<br>
	 * <b>expected</b>: Data written to buffer, TXEIE enabled.<br>
    * ************************************************
	 */
	char data2[]= "SECOND_STRING\n";
	EXPECT_EQ(RETURN_OK, uartengine_send_string(data2));
	EXPECT_EQ(uart_tx_buf.head, 9);
	EXPECT_EQ(uart_tx_buf.tail, 15);
	EXPECT_EQ(uart_tx_buf.buf[15], 'S');
	EXPECT_EQ(uart_tx_buf.buf[16], 'E');
	EXPECT_EQ(uart_tx_buf.buf[17], 'C');
	EXPECT_EQ(uart_tx_buf.buf[18], 'O');
	EXPECT_EQ(uart_tx_buf.buf[19], 'N');
	EXPECT_EQ(uart_tx_buf.buf[0], 'D');
	EXPECT_EQ(uart_tx_buf.buf[1], '_');
	EXPECT_EQ(uart_tx_buf.buf[2], 'S');
	EXPECT_EQ(uart_tx_buf.buf[3], 'T');
	EXPECT_EQ(uart_tx_buf.buf[4], 'R');
	EXPECT_EQ(uart_tx_buf.buf[5], 'I');
	EXPECT_EQ(uart_tx_buf.buf[6], 'N');
	EXPECT_EQ(uart_tx_buf.buf[7], 'G');
	EXPECT_EQ(uart_tx_buf.buf[8], '\n');
	EXPECT_TRUE(READ_BIT(USART2->CR1, USART_CR1_TXEIE));

	/* Simulate USART IRQ - read data from buffer */
	USART2->SR |=  USART_SR_TXE;
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'S');
	EXPECT_EQ(uart_tx_buf.tail, 16);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'E');
	EXPECT_EQ(uart_tx_buf.tail, 17);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'C');
	EXPECT_EQ(uart_tx_buf.tail, 18);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'O');
	EXPECT_EQ(uart_tx_buf.tail, 19);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'N');
	EXPECT_EQ(uart_tx_buf.tail, 0);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'D');
	EXPECT_EQ(uart_tx_buf.tail, 1);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, '_');
	EXPECT_EQ(uart_tx_buf.tail, 2);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'S');
	EXPECT_EQ(uart_tx_buf.tail, 3);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'T');
	EXPECT_EQ(uart_tx_buf.tail, 4);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'R');
	EXPECT_EQ(uart_tx_buf.tail, 5);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'I');
	EXPECT_EQ(uart_tx_buf.tail, 6);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'N');
	EXPECT_EQ(uart_tx_buf.tail, 7);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'G');
	EXPECT_EQ(uart_tx_buf.tail, 8);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, '\n');
	EXPECT_EQ(uart_tx_buf.tail, 9);
	USART2_IRQHandler();
	EXPECT_EQ(uart_tx_buf.tail, uart_tx_buf.head);
	EXPECT_FALSE(READ_BIT(USART2->CR1, USART_CR1_TXEIE));

	/**
	 * <b>scenario</b>: String send with CR LF ending.<br>
	 * <b>expected</b>: Data written to buffer, TXEIE enabled.<br>
    * ************************************************
	 */
	char data3[]= "AT_CMD\r\n";
	EXPECT_EQ(RETURN_OK, uartengine_send_string(data3));
	EXPECT_EQ(uart_tx_buf.head, 17);
	EXPECT_EQ(uart_tx_buf.tail, 9);
	EXPECT_EQ(uart_tx_buf.buf[9], 'A');
	EXPECT_EQ(uart_tx_buf.buf[10], 'T');
	EXPECT_EQ(uart_tx_buf.buf[11], '_');
	EXPECT_EQ(uart_tx_buf.buf[12], 'C');
	EXPECT_EQ(uart_tx_buf.buf[13], 'M');
	EXPECT_EQ(uart_tx_buf.buf[14], 'D');
	EXPECT_EQ(uart_tx_buf.buf[15], '\r');
	EXPECT_EQ(uart_tx_buf.buf[16], '\n');
	EXPECT_TRUE(READ_BIT(USART2->CR1, USART_CR1_TXEIE));

	/* Simulate USART IRQ - read data from buffer */
	USART2->SR |=  USART_SR_TXE;
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'A');
	EXPECT_EQ(uart_tx_buf.tail, 10);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'T');
	EXPECT_EQ(uart_tx_buf.tail, 11);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, '_');
	EXPECT_EQ(uart_tx_buf.tail, 12);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'C');
	EXPECT_EQ(uart_tx_buf.tail, 13);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'M');
	EXPECT_EQ(uart_tx_buf.tail, 14);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, 'D');
	EXPECT_EQ(uart_tx_buf.tail, 15);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, '\r');
	EXPECT_EQ(uart_tx_buf.tail, 16);
	USART2_IRQHandler();
	EXPECT_EQ(USART2->DR, '\n');
	EXPECT_EQ(uart_tx_buf.tail, 17);
	USART2_IRQHandler();

	EXPECT_EQ(uart_tx_buf.tail, uart_tx_buf.head);
	EXPECT_FALSE(READ_BIT(USART2->CR1, USART_CR1_TXEIE));

	uartengine_deinitialize();
}

/**
 * @test Reading string from UART
 */
TEST_F(uartengineFixture, string_read)
{
	/**
	 * <b>scenario</b>: String read.<br>
	 * <b>expected</b>: Data written to provided buffer, NULL terminated.<br>
    * ************************************************
	 */
	UARTEngine_Config cfg = {115200, 20, 15};
	EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(2);
	uartengine_initialize(&cfg);
	uartengine_register_callback(&fake_callback);

	USART2->SR |= USART_SR_RXNE;

	USART2->DR = 'R';
	USART2_IRQHandler();
	USART2->DR = 'E';
	USART2_IRQHandler();
	USART2->DR = 'A';
	USART2_IRQHandler();
	USART2->DR = 'D';
	USART2_IRQHandler();
	USART2->DR = '_';
	USART2_IRQHandler();
	USART2->DR = 'S';
	USART2_IRQHandler();
	USART2->DR = 'T';
	USART2_IRQHandler();
	USART2->DR = 'R';
	USART2_IRQHandler();
	USART2->DR = 'I';
	USART2_IRQHandler();
	USART2->DR = 'N';
	USART2_IRQHandler();
	USART2->DR = 'G';
	USART2_IRQHandler();
	USART2->DR = '\n';
	USART2_IRQHandler();
	EXPECT_EQ(uart_rx_buf.head, 12);

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
	EXPECT_EQ(uart_rx_buf.head, uart_rx_buf.tail);

	USART2->DR = 'S';
	USART2_IRQHandler();
	USART2->DR = 'E';
	USART2_IRQHandler();
	USART2->DR = 'C';
	USART2_IRQHandler();
	USART2->DR = 'O';
	USART2_IRQHandler();
	USART2->DR = 'N';
	USART2_IRQHandler();
	USART2->DR = 'D';
	USART2_IRQHandler();
	USART2->DR = '_';
	USART2_IRQHandler();
	USART2->DR = 'R';
	USART2_IRQHandler();
	USART2->DR = 'E';
	USART2_IRQHandler();
	USART2->DR = 'A';
	USART2_IRQHandler();
	USART2->DR = 'D';
	USART2_IRQHandler();
	USART2->DR = '\n';
	USART2_IRQHandler();
	EXPECT_EQ(uart_rx_buf.head, 4);

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
	EXPECT_EQ(uart_rx_buf.head, uart_rx_buf.tail);


	USART2->DR = 'A';
	USART2_IRQHandler();
	USART2->DR = 'T';
	USART2_IRQHandler();
	USART2->DR = '_';
	USART2_IRQHandler();
	USART2->DR = 'R';
	USART2_IRQHandler();
	USART2->DR = 'E';
	USART2_IRQHandler();
	USART2->DR = 'P';
	USART2_IRQHandler();
	USART2->DR = 'L';
	USART2_IRQHandler();
	USART2->DR = 'Y';
	USART2_IRQHandler();
	USART2->DR = '\r';
	USART2_IRQHandler();
	USART2->DR = '\n';
	USART2_IRQHandler();
	EXPECT_EQ(uart_rx_buf.head, 14);

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
	EXPECT_EQ(uart_rx_buf.head, uart_rx_buf.tail);

	uartengine_deinitialize();
}

/**
 * @test Mixed read (string and bytes) from UART
 */
TEST_F(uartengineFixture, string_bytes_read_mixed)
{
	/**
	 * <b>scenario</b>: String received.<br>
	 * <b>expected</b>: String written to external buffer, internal buffer indexes are equal.<br>
    * ************************************************
	 */
	UARTEngine_Config cfg = {115200, 20, 15};
	EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(2);
	uartengine_initialize(&cfg);
	uartengine_register_callback(&fake_callback);

	USART2->SR |= USART_SR_RXNE;

	USART2->DR = 'R';
	USART2_IRQHandler();
	USART2->DR = 'E';
	USART2_IRQHandler();
	USART2->DR = 'A';
	USART2_IRQHandler();
	USART2->DR = 'D';
	USART2_IRQHandler();
	USART2->DR = '_';
	USART2_IRQHandler();
	USART2->DR = 'S';
	USART2_IRQHandler();
	USART2->DR = 'T';
	USART2_IRQHandler();
	USART2->DR = 'R';
	USART2_IRQHandler();
	USART2->DR = 'I';
	USART2_IRQHandler();
	USART2->DR = 'N';
	USART2_IRQHandler();
	USART2->DR = 'G';
	USART2_IRQHandler();
	USART2->DR = '\n';
	USART2_IRQHandler();
	EXPECT_EQ(uart_rx_buf.head, 12);
	EXPECT_EQ(uart_rx_buf.tail, 0);
	EXPECT_EQ(RETURN_OK, uartengine_can_read_string());
	EXPECT_STREQ("READ_STRING", uartengine_get_string());
	EXPECT_EQ(uart_rx_buf.head, uart_rx_buf.tail);

	/**
	 * <b>scenario</b>: String received.<br>
	 * <b>expected</b>: String written to external buffer, internal buffer indexes are equal.<br>
    * ************************************************
	 */
	USART2->DR = '\r';
	USART2_IRQHandler();
	USART2->DR = '\n';
	USART2_IRQHandler();
	USART2->DR = 'S';
	USART2_IRQHandler();
	USART2->DR = 'E';
	USART2_IRQHandler();
	USART2->DR = 'C';
	USART2_IRQHandler();
	USART2->DR = '_';
	USART2_IRQHandler();
	USART2->DR = 'S';
	USART2_IRQHandler();
	USART2->DR = 'T';
	USART2_IRQHandler();
	USART2->DR = 'R';
	USART2_IRQHandler();
	USART2->DR = 'I';
	USART2_IRQHandler();
	USART2->DR = 'N';
	USART2_IRQHandler();
	USART2->DR = 'G';
	USART2_IRQHandler();
	USART2->DR = '\r';
	USART2_IRQHandler();
	USART2->DR = '\n';
	USART2_IRQHandler();

	EXPECT_EQ(2, uart_rx_buf.string_cnt);
	EXPECT_EQ(RETURN_NOK, uartengine_can_read_string());
	EXPECT_EQ(1, uart_rx_buf.string_cnt);
	EXPECT_EQ(RETURN_OK, uartengine_can_read_string());
	EXPECT_STREQ("SEC_STRING", uartengine_get_string());
	EXPECT_EQ(uart_rx_buf.head, uart_rx_buf.tail);

	uartengine_deinitialize();
}

/**
 * @test Adding/removing callbacks tests
 */
TEST_F(uartengineFixture, callback_add_remove)
{

	for (uint8_t i = 0; i < UART_ENGINE_CALLBACK_SIZE; i++)
	{
		UART_CALLBACKS[i] = NULL;
	}
	/**
	 * <b>scenario</b>: Callback list empty - adding one callback.<br>
	 * <b>expected</b>: Callback placed on first place.<br>
    * ************************************************
	 */
	EXPECT_EQ(RETURN_OK, uartengine_register_callback(&fake_callback));
	EXPECT_CALL(*callMock, callback(_));
	uartengine_notify_callbacks();

	/**
	 * <b>scenario</b>: Callback list full, calling callbacks.<br>
	 * <b>expected</b>: All callbacks called.<br>
    * ************************************************
	 */
	for (uint8_t i = 1; i < UART_ENGINE_CALLBACK_SIZE; i++)
	{
		EXPECT_EQ(RETURN_OK, uartengine_register_callback(&fake_callback));
	}
	EXPECT_CALL(*callMock, callback(_)).Times(5);
	uartengine_notify_callbacks();
	/**
	 * <b>scenario</b>: Callback list full, adding new callback.<br>
	 * <b>expected</b>: New callback not added.<br>
    * ************************************************
	 */
	EXPECT_EQ(RETURN_ERROR, uartengine_register_callback(&fake_callback));
	EXPECT_CALL(*callMock, callback(_)).Times(5);
	uartengine_notify_callbacks();

	/**
	 * <b>scenario</b>: Callback list full, unregister callback.<br>
	 * <b>expected</b>: Callback unregistered<br>
    * ************************************************
	 */
	EXPECT_EQ(RETURN_OK, uartengine_unregister_callback(&fake_callback));
	EXPECT_CALL(*callMock, callback(_)).Times(4);
	uartengine_notify_callbacks();

	/**
	 * <b>scenario</b>: Callback list not full, unregister 2 more callbacks.
	 * <b>expected</b>: Callback unregistered<br>
    * ************************************************
	 */
	EXPECT_EQ(RETURN_OK, uartengine_unregister_callback(&fake_callback));
	EXPECT_EQ(RETURN_OK, uartengine_unregister_callback(&fake_callback));
	EXPECT_CALL(*callMock, callback(_)).Times(2);
	uartengine_notify_callbacks();

	/**
	 * <b>scenario</b>: Callback list not contiguous, adding new callback.<br>
	 * <b>expected</b>: Callback added<br>
    * ************************************************
	 */
	EXPECT_EQ(RETURN_OK, uartengine_register_callback(&fake_callback));
	EXPECT_CALL(*callMock, callback(_)).Times(3);
	uartengine_notify_callbacks();

	for (uint8_t i = 0; i < UART_ENGINE_CALLBACK_SIZE; i++)
	{
		UART_CALLBACKS[i] = NULL;
	}
}

/**
 * @test Reading string from UART when unexpected line endings (CR, LF) occurs.
 */
TEST_F(uartengineFixture, string_read_special_data)
{
	/**
	 * <b>scenario</b>: CRLF sequence received on begin and end of the string.<br>
	 * <b>expected</b>: CRLF sequences shall be filtered out.<br>
    * ************************************************
	 */
	UARTEngine_Config cfg = {115200, 20, 15};
	EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(2);
	uartengine_initialize(&cfg);
	uartengine_register_callback(&fake_callback);

	USART2->SR |= USART_SR_RXNE;

	USART2->DR = '\r';
	USART2_IRQHandler();
	USART2->DR = '\n';
	USART2_IRQHandler();
	USART2->DR = 'O';
	USART2_IRQHandler();
	USART2->DR = 'K';
	USART2_IRQHandler();
	USART2->DR = '\r';
	USART2_IRQHandler();
	USART2->DR = '\n';
	USART2_IRQHandler();
	EXPECT_EQ(uart_rx_buf.head, 6);
	EXPECT_EQ(uart_rx_buf.tail, 0);
	EXPECT_EQ(uart_rx_buf.string_cnt, 2);

	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](const char* buf)
			{
				EXPECT_EQ(buf[0], 'O');
				EXPECT_EQ(buf[1], 'K');
				EXPECT_EQ(buf[2], '\0');
			}));

	uartengine_string_watcher(); //shouldn't call callback, because first string is empty
	uartengine_string_watcher();
	EXPECT_EQ(uart_rx_buf.head, uart_rx_buf.tail);
	EXPECT_EQ(uart_rx_buf.string_cnt, 0);

	USART2->DR = '\r';
	USART2_IRQHandler();
	USART2->DR = '\n';
	USART2_IRQHandler();
	USART2->DR = 'O';
	USART2_IRQHandler();
	USART2->DR = 'K';
	USART2_IRQHandler();
	USART2->DR = '\r';
	USART2_IRQHandler();
	USART2->DR = '\n';
	USART2_IRQHandler();
	EXPECT_EQ(uart_rx_buf.head, 12);
	EXPECT_EQ(uart_rx_buf.tail, 6);
	EXPECT_EQ(uart_rx_buf.string_cnt, 2);

	EXPECT_CALL(*callMock, callback(_)).WillOnce(Invoke([&](const char* buf)
			{
				EXPECT_EQ(buf[0], 'O');
				EXPECT_EQ(buf[1], 'K');
				EXPECT_EQ(buf[2], '\0');
			}));

	uartengine_string_watcher(); //shouldn't call callback, because first string is empty
	uartengine_string_watcher();
	EXPECT_EQ(uart_rx_buf.head, uart_rx_buf.tail);
	EXPECT_EQ(uart_rx_buf.string_cnt, 0);

	uartengine_deinitialize();
}

/**
 * @test Reading string from UART when unexpected line endings (CR, LF) occurs when polling on string.
 */
TEST_F(uartengineFixture, string_read_special_data_pooling)
{
	/**
	 * <b>scenario</b>: CRLF sequence received on begin and end of the string.<br>
	 * <b>expected</b>: CRLF sequences shall be filtered out.<br>
    * ************************************************
	 */
	UARTEngine_Config cfg = {115200, 20, 15};
	EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(2);
	uartengine_initialize(&cfg);
	uartengine_register_callback(&fake_callback);

	USART2->SR |= USART_SR_RXNE;

	USART2->DR = '\r';
	USART2_IRQHandler();
	EXPECT_EQ(RETURN_NOK, uartengine_can_read_string());
	EXPECT_EQ(uart_rx_buf.head, 1);
	EXPECT_EQ(uart_rx_buf.tail, 0);

	USART2->DR = '\n';
	USART2_IRQHandler();
	EXPECT_EQ(RETURN_NOK, uartengine_can_read_string());
	EXPECT_EQ(uart_rx_buf.head, 2);
	EXPECT_EQ(uart_rx_buf.tail, 2);

	USART2->DR = 'O';
	USART2_IRQHandler();
	EXPECT_EQ(RETURN_NOK, uartengine_can_read_string());
	EXPECT_EQ(uart_rx_buf.head, 3);
	EXPECT_EQ(uart_rx_buf.tail, 2);

	USART2->DR = 'K';
	USART2_IRQHandler();
	EXPECT_EQ(RETURN_NOK, uartengine_can_read_string());
	EXPECT_EQ(uart_rx_buf.head, 4);
	EXPECT_EQ(uart_rx_buf.tail, 2);

	USART2->DR = '\r';
	USART2_IRQHandler();
	EXPECT_EQ(RETURN_NOK, uartengine_can_read_string());
	EXPECT_EQ(uart_rx_buf.head, 5);
	EXPECT_EQ(uart_rx_buf.tail, 2);

	USART2->DR = '\n';
	USART2_IRQHandler();
	EXPECT_EQ(RETURN_OK, uartengine_can_read_string());
	EXPECT_EQ(uart_rx_buf.head, 6);
	EXPECT_EQ(uart_rx_buf.tail, 6);

	EXPECT_STREQ("OK", uartengine_get_string());

	uartengine_deinitialize();
}

/**
 * @test Reading raw bytes from UART.
 */
TEST_F(uartengineFixture, string_read_bytes)
{
	/**
	 * <b>scenario</b>: 10 bytes received, includes CR and LR chars.<br>
	 * <b>expected</b>: All bytes can be read.<br>
    * ************************************************
	 */
	UARTEngine_Config cfg = {115200, 15, 15};
	EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(2);
	uartengine_initialize(&cfg);
	uartengine_register_callback(&fake_callback);

	USART2->SR |= USART_SR_RXNE;

	USART2->DR = 0x01;
	USART2_IRQHandler();
	EXPECT_EQ(1U, uartengine_count_bytes());

	USART2->DR = 0x02;
	USART2_IRQHandler();
	EXPECT_EQ(2U, uartengine_count_bytes());

	USART2->DR = 0x10; // '\n' char
	USART2_IRQHandler();
	EXPECT_EQ(3U, uartengine_count_bytes());

	USART2->DR = 0x03;
	USART2_IRQHandler();
	EXPECT_EQ(4U, uartengine_count_bytes());

	USART2->DR = 0x13; // '\r' char
	USART2_IRQHandler();
	EXPECT_EQ(5U, uartengine_count_bytes());

	USART2->DR = 0x04;
	USART2_IRQHandler();
	EXPECT_EQ(6U, uartengine_count_bytes());

	USART2->DR = 0x01;
	USART2_IRQHandler();
	EXPECT_EQ(7U, uartengine_count_bytes());

	USART2->DR = 0x02;
	USART2_IRQHandler();
	EXPECT_EQ(8U, uartengine_count_bytes());

	USART2->DR = 0x10; // '\n' char
	USART2_IRQHandler();
	EXPECT_EQ(9U, uartengine_count_bytes());

	USART2->DR = 0x03;
	USART2_IRQHandler();
	EXPECT_EQ(10U, uartengine_count_bytes());

	EXPECT_EQ(uart_rx_buf.head, 10);
	EXPECT_EQ(uart_rx_buf.tail, 0);

	const uint8_t* result = uartengine_get_bytes();

	EXPECT_EQ(result[0], 0x01);
	EXPECT_EQ(result[1], 0x02);
	EXPECT_EQ(result[2], 0x10);
	EXPECT_EQ(result[3], 0x03);
	EXPECT_EQ(result[4], 0x13);
	EXPECT_EQ(result[5], 0x04);
	EXPECT_EQ(result[6], 0x01);
	EXPECT_EQ(result[7], 0x02);
	EXPECT_EQ(result[8], 0x10);
	EXPECT_EQ(result[9], 0x03);

	EXPECT_EQ(uart_rx_buf.head, uart_rx_buf.tail);

	USART2->DR = 0x11;
	USART2_IRQHandler();
	EXPECT_EQ(1U, uartengine_count_bytes());

	USART2->DR = 0x12; // '\r' char
	USART2_IRQHandler();
	EXPECT_EQ(2U, uartengine_count_bytes());

	USART2->DR = 0x13;
	USART2_IRQHandler();
	EXPECT_EQ(3U, uartengine_count_bytes());

	USART2->DR = 0x14;
	USART2_IRQHandler();
	EXPECT_EQ(4U, uartengine_count_bytes());

	USART2->DR = 0x15;
	USART2_IRQHandler();
	EXPECT_EQ(5U, uartengine_count_bytes());

	USART2->DR = 0x16; // '\n' char
	USART2_IRQHandler();
	EXPECT_EQ(6U, uartengine_count_bytes());

	USART2->DR = 0x17;
	USART2_IRQHandler();
	EXPECT_EQ(7U, uartengine_count_bytes());

	EXPECT_EQ(uart_rx_buf.head, 2);
	EXPECT_EQ(uart_rx_buf.tail, 10);

	const uint8_t* result2 = uartengine_get_bytes();

	EXPECT_EQ(result2[0], 0x11);
	EXPECT_EQ(result2[1], 0x12);
	EXPECT_EQ(result2[2], 0x13);
	EXPECT_EQ(result2[3], 0x14);
	EXPECT_EQ(result2[4], 0x15);
	EXPECT_EQ(result2[5], 0x16);
	EXPECT_EQ(result2[6], 0x17);

	EXPECT_EQ(uart_rx_buf.head, uart_rx_buf.tail);

	uartengine_deinitialize();

}

/**
 * @test Sending raw bytes to UART.
 */
TEST_F(uartengineFixture, uart_send_bytes)
{
   const uint8_t data_size = 10;
   uint8_t data_to_send [data_size];
   data_to_send[0] = 0x10;
   data_to_send[1] = 0x11;
   data_to_send[2] = 0x12;
   data_to_send[3] = 0x13;
   data_to_send[4] = 0x14;
   data_to_send[5] = 0x15;
   data_to_send[6] = 0x16;
   data_to_send[7] = 0x17;
   data_to_send[8] = 0x18;
   data_to_send[9] = 0x19;

   /**
    * <b>scenario</b>: Request to send 10 bytes.<br>
    * <b>expected</b>: 10 bytes written correctly.<br>
    * ************************************************
    */
   UARTEngine_Config cfg = {115200, 20, 15};
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(2);
   uartengine_initialize(&cfg);

   EXPECT_EQ(RETURN_OK, uartengine_send_bytes(data_to_send, data_size));
   EXPECT_EQ(uart_tx_buf.head, 10);
   EXPECT_EQ(uart_tx_buf.tail, 0);
   EXPECT_EQ(uart_tx_buf.buf[0], 0x10);
   EXPECT_EQ(uart_tx_buf.buf[1], 0x11);
   EXPECT_EQ(uart_tx_buf.buf[2], 0x12);
   EXPECT_EQ(uart_tx_buf.buf[3], 0x13);
   EXPECT_EQ(uart_tx_buf.buf[4], 0x14);
   EXPECT_EQ(uart_tx_buf.buf[5], 0x15);
   EXPECT_EQ(uart_tx_buf.buf[6], 0x16);
   EXPECT_EQ(uart_tx_buf.buf[7], 0x17);
   EXPECT_EQ(uart_tx_buf.buf[8], 0x18);
   EXPECT_EQ(uart_tx_buf.buf[9], 0x19);
   EXPECT_TRUE(READ_BIT(USART2->CR1, USART_CR1_TXEIE));

   /* Simulate USART IRQ - read data from buffer */
   USART2->SR |=  USART_SR_TXE;
   USART2_IRQHandler();
   EXPECT_EQ(USART2->DR, 0x10);
   EXPECT_EQ(uart_tx_buf.tail, 1);
   USART2_IRQHandler();
   EXPECT_EQ(USART2->DR, 0x11);
   EXPECT_EQ(uart_tx_buf.tail, 2);
   USART2_IRQHandler();
   EXPECT_EQ(USART2->DR, 0x12);
   EXPECT_EQ(uart_tx_buf.tail, 3);
   USART2_IRQHandler();
   EXPECT_EQ(USART2->DR, 0x13);
   EXPECT_EQ(uart_tx_buf.tail, 4);
   USART2_IRQHandler();
   EXPECT_EQ(USART2->DR, 0x14);
   EXPECT_EQ(uart_tx_buf.tail, 5);
   USART2_IRQHandler();
   EXPECT_EQ(USART2->DR, 0x15);
   EXPECT_EQ(uart_tx_buf.tail, 6);
   USART2_IRQHandler();
   EXPECT_EQ(USART2->DR, 0x16);
   EXPECT_EQ(uart_tx_buf.tail, 7);
   USART2_IRQHandler();
   EXPECT_EQ(USART2->DR, 0x17);
   EXPECT_EQ(uart_tx_buf.tail, 8);
   USART2_IRQHandler();
   EXPECT_EQ(USART2->DR, 0x18);
   EXPECT_EQ(uart_tx_buf.tail, 9);
   USART2_IRQHandler();
   EXPECT_EQ(USART2->DR, 0x19);
   EXPECT_EQ(uart_tx_buf.tail, 10);
   USART2_IRQHandler();

   EXPECT_EQ(uart_tx_buf.tail, uart_tx_buf.head);
   EXPECT_FALSE(READ_BIT(USART2->CR1, USART_CR1_TXEIE));

   uartengine_deinitialize();
}
