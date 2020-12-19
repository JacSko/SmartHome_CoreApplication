#include "gtest/gtest.h"
#include <thread>
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
   MOCK_METHOD4(callback, void(I2C_OP_TYPE, I2C_STATUS, const uint8_t*, uint8_t));
};

callbackMock* callMock;

void fake_callback(I2C_OP_TYPE type, I2C_STATUS status, const uint8_t* data, uint8_t size)
{
   callMock->callback(type, status, data, size);
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
      EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_,TASKPRIO_HIGH,I2C_DEFAULT_TIMEOUT_MS,_,_))
      .WillOnce(Return(RETURN_OK));
      EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_, _, _)).Times(2);
      i2c_initialize();
   }

   virtual void TearDown()
   {
      i2c_deinitialize();
      mock_logger_deinit();
      stm_stub_deinit();
      mock_gpio_deinit();
      mock_sch_deinit();
      delete callMock;
   }
   void simulate_SB_interrupt()
   {
      I2C1->SR1 |= I2C_SR1_SB;
      I2C1_EV_IRQHandler();
      I2C1->SR1 &= ~I2C_SR1_SB;
   }
   void simulate_ADDR_interrupt()
   {
      I2C1->SR1 |= I2C_SR1_ADDR;
      I2C1_EV_IRQHandler();
      I2C1->SR1 &= ~I2C_SR1_ADDR;
   }
   void simulate_BTF_interrupt()
   {
      I2C1->SR1 |= I2C_SR1_BTF;
      I2C1_EV_IRQHandler();
      I2C1->SR1 &= ~I2C_SR1_BTF;
   }
   void simulate_RXNE_interrupt(uint8_t data)
   {
      I2C1->SR1 |= I2C_SR1_RXNE;
      I2C1->DR = data;
      I2C1_EV_IRQHandler();
      I2C1->SR1 &= ~I2C_SR1_RXNE;
   }
};

/**
 * @test Writing data to I2C bus - asynchronous, correct sequence
 */
TEST_F(i2cDriverFixture, i2c_async_write_test)
{
   EXPECT_CALL(*logger_mock, logger_get_group_state(_)).WillOnce(Return(LOGGER_GROUP_ENABLE));
   uint8_t test_buffer [16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
   uint8_t test_address = 0xFF;
   /**
    * <b>scenario</b>:  Try to write more data than possible <br>
    * <b>expected</b>:  RETURN_NOK returned, transaction not started <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(0);
   EXPECT_EQ(RETURN_NOK, i2c_write_async(test_address, test_buffer, I2C_DRV_BUFFER_SIZE+1, NULL));

   /**
    * <b>scenario</b>:  Async write to I2C bus - correct sequence <br>
    * <b>expected</b>:  RETURN_OK returned, transaction started <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(1);
   EXPECT_EQ(RETURN_OK, i2c_write_async(test_address, test_buffer, 3, &fake_callback));

   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_START) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_ACK) != 0);

   simulate_SB_interrupt();
   EXPECT_EQ(I2C1->DR, test_address);

   simulate_ADDR_interrupt();
   EXPECT_EQ(I2C1->DR, test_buffer[0]);

   for (uint8_t i = 1; i < 3; i++)
   {
      simulate_BTF_interrupt();
      EXPECT_EQ(I2C1->DR, test_buffer[i]);
   }

   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).Times(1);
   simulate_BTF_interrupt();
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_STOP) != 0);

   EXPECT_CALL(*callMock, callback(I2C_OP_WRITE,I2C_STATUS_OK,_,3));
   i2c_watcher();

}

/**
 * @test Writing data to I2C bus - synchronous, correct sequence
 */
TEST_F(i2cDriverFixture, i2c_sync_write_test)
{
   EXPECT_CALL(*logger_mock, logger_get_group_state(_)).WillOnce(Return(LOGGER_GROUP_ENABLE));
   uint8_t test_buffer [16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
   uint8_t test_address = 0xFF;
   /**
    * <b>scenario</b>:  Sync write to I2C bus - correct sequence <br>
    * <b>expected</b>:  RETURN_OK returned, transaction started <br>
    * ************************************************
    */

   std::thread thread([&]()
            {
               while(i2c_driver.state != I2C_STATE_STARTED);
               EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_START) != 0);
               EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_ACK) != 0);
               simulate_SB_interrupt();
               EXPECT_EQ(I2C1->DR, test_address);

               simulate_ADDR_interrupt();
               EXPECT_EQ(I2C1->DR, test_buffer[0]);

               for (uint8_t i = 1; i < 3; i++)
               {
                  simulate_BTF_interrupt();
                  EXPECT_EQ(I2C1->DR, test_buffer[i]);
               }

               EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).Times(1);
               simulate_BTF_interrupt();
               EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_STOP) != 0);

            });

   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(1);

   EXPECT_EQ(I2C_STATUS_OK, i2c_write(test_address, test_buffer, 3));

   thread.join();

}

/**
 * @test Writing data to I2C bus - asynchronous, correct sequence, one byte to write
 */
TEST_F(i2cDriverFixture, i2c_async_write_one_byte_test)
{
   EXPECT_CALL(*logger_mock, logger_get_group_state(_)).WillOnce(Return(LOGGER_GROUP_ENABLE));
   uint8_t test_buffer [16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
   uint8_t test_address = 0xFF;
   /**
    * <b>scenario</b>:  One byte async write to I2C bus - correct sequence <br>
    * <b>expected</b>:  RETURN_OK returned, transaction started <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(1);
   EXPECT_EQ(RETURN_OK, i2c_write_async(test_address, test_buffer, 1, &fake_callback));

   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_START) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_ACK) != 0);

   simulate_SB_interrupt();
   EXPECT_EQ(I2C1->DR, test_address);

   simulate_ADDR_interrupt();
   EXPECT_EQ(I2C1->DR, test_buffer[0]);

   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).Times(1);
   simulate_BTF_interrupt();
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_STOP) != 0);

   EXPECT_CALL(*callMock, callback(I2C_OP_WRITE,I2C_STATUS_OK,_,1));
   i2c_watcher();

}


/**
 * @test Writing data to I2C bus - asynchronous, timeout occurs
 */
TEST_F(i2cDriverFixture, i2c_async_write_timeout_test)
{
   EXPECT_CALL(*logger_mock, logger_get_group_state(_)).WillOnce(Return(LOGGER_GROUP_ENABLE));
   uint8_t test_buffer [16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
   uint8_t test_address = 0xFF;

   /**
    * <b>scenario</b>:  Async write to I2C bus - timeout after address sent <br>
    * <b>expected</b>:  RETURN_OK returned, transaction started, callback with error called <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(1);
   EXPECT_EQ(RETURN_OK, i2c_write_async(test_address, test_buffer, 3, &fake_callback));

   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_START) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_ACK) != 0);

   simulate_SB_interrupt();
   EXPECT_EQ(I2C1->DR, test_address);

   simulate_ADDR_interrupt();
   EXPECT_EQ(I2C1->DR, test_buffer[0]);

   i2c_on_timeout();


   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_, _, _)).Times(2);
   EXPECT_CALL(*callMock, callback(I2C_OP_WRITE,I2C_STATUS_ERROR,_,_));
   i2c_watcher();

   /**
    * <b>scenario</b>:  Async write to I2C bus - reading after timeout <br>
    * <b>expected</b>:  RETURN_OK returned, transaction started <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(1);
   EXPECT_EQ(RETURN_OK, i2c_write_async(test_address, test_buffer, 3, &fake_callback));

   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_START) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_ACK) != 0);

   simulate_SB_interrupt();
   EXPECT_EQ(I2C1->DR, test_address);

   simulate_ADDR_interrupt();
   EXPECT_EQ(I2C1->DR, test_buffer[0]);

   for (uint8_t i = 1; i < 3; i++)
   {
      simulate_BTF_interrupt();
      EXPECT_EQ(I2C1->DR, test_buffer[i]);
   }

   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).Times(1);
   simulate_BTF_interrupt();
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_STOP) != 0);

   EXPECT_CALL(*callMock, callback(I2C_OP_WRITE,I2C_STATUS_OK,_,3));
   i2c_watcher();


}

/**
 * @test Reading data from I2C bus - asynchronous, correct sequence
 */
TEST_F(i2cDriverFixture, i2c_async_read_test)
{
   EXPECT_CALL(*logger_mock, logger_get_group_state(_)).WillOnce(Return(LOGGER_GROUP_ENABLE));
   uint8_t test_address = 0xFF;
   /**
    * <b>scenario</b>:  Try to read more data than possible <br>
    * <b>expected</b>:  RETURN_NOK returned, transaction not started <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(0);
   EXPECT_EQ(RETURN_NOK, i2c_read_async(test_address, I2C_DRV_BUFFER_SIZE+1, NULL));

   /**
    * <b>scenario</b>:  Async read from I2C bus - correct sequence <br>
    * <b>expected</b>:  RETURN_OK returned, transaction started <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(1);
   EXPECT_EQ(RETURN_OK, i2c_read_async(test_address, 3, &fake_callback));

   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_START) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_ACK) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR2_ITBUFEN) != 0);

   simulate_SB_interrupt();
   EXPECT_EQ(I2C1->DR, test_address);

   simulate_ADDR_interrupt();

   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).Times(1);
   for (uint8_t i = 0; i < 3; i++)
   {
      simulate_RXNE_interrupt(i);
   }
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_STOP) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR2_ITBUFEN) == 0);

   EXPECT_CALL(*callMock, callback(I2C_OP_READ,I2C_STATUS_OK,_,3)).WillOnce(Invoke([&]
   (I2C_OP_TYPE, I2C_STATUS, const uint8_t* data, uint8_t)
   {
      EXPECT_EQ(data[0], 0x00);
      EXPECT_EQ(data[1], 0x01);
      EXPECT_EQ(data[2], 0x02);
   }));
   i2c_watcher();
}

/**
 * @test Reading data from I2C bus - synchronous, correct sequence
 */
TEST_F(i2cDriverFixture, i2c_sync_read_test)
{
   EXPECT_CALL(*logger_mock, logger_get_group_state(_)).WillOnce(Return(LOGGER_GROUP_ENABLE));
   uint8_t test_address = 0xFF;
   uint8_t test_buffer [3] = {0x00, 0x00, 0x00};
   /**
    * <b>scenario</b>:  Sync read from I2C bus - correct sequence <br>
    * <b>expected</b>:  RETURN_OK returned, transaction started <br>
    * ************************************************
    */

   std::thread thread([&]()
            {
               while(i2c_driver.state != I2C_STATE_STARTED);
               EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_START) != 0);
               EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_ACK) != 0);
               EXPECT_TRUE( (I2C1->CR1 & I2C_CR2_ITBUFEN) != 0);

               simulate_SB_interrupt();
               EXPECT_EQ(I2C1->DR, test_address);

               simulate_ADDR_interrupt();

               EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).Times(1);
               for (uint8_t i = 0; i < 3; i++)
               {
                  simulate_RXNE_interrupt(i);
               }
               EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_STOP) != 0);
               EXPECT_TRUE( (I2C1->CR1 & I2C_CR2_ITBUFEN) == 0);

            });

   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(1);
   EXPECT_EQ(I2C_STATUS_OK, i2c_read(test_address, test_buffer, 3));
   EXPECT_EQ(test_buffer[0], 0x00);
   EXPECT_EQ(test_buffer[1], 0x01);
   EXPECT_EQ(test_buffer[2], 0x02);
   thread.join();

}


/**
 * @test Reading 1 byte from I2C bus - asynchronous, correct sequence
 */
TEST_F(i2cDriverFixture, i2c_async_read_one_byte_test)
{
   EXPECT_CALL(*logger_mock, logger_get_group_state(_)).WillOnce(Return(LOGGER_GROUP_ENABLE));
   uint8_t test_address = 0xFF;

   /**
    * <b>scenario</b>:  1 byte async read from I2C bus - correct sequence <br>
    * <b>expected</b>:  RETURN_OK returned, transaction started <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(1);
   EXPECT_EQ(RETURN_OK, i2c_read_async(test_address, 1, &fake_callback));

   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_START) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_ACK) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR2_ITBUFEN) != 0);

   simulate_SB_interrupt();
   EXPECT_EQ(I2C1->DR, test_address);

   simulate_ADDR_interrupt();
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).Times(1);
   simulate_RXNE_interrupt(0x11);

   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_STOP) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR2_ITBUFEN) == 0);

   EXPECT_CALL(*callMock, callback(I2C_OP_READ,I2C_STATUS_OK,_,1)).WillOnce(Invoke([&]
   (I2C_OP_TYPE, I2C_STATUS, const uint8_t* data, uint8_t)
   {
      EXPECT_EQ(data[0], 0x11);
   }));
   i2c_watcher();
}

/**
 * @test Reading data from I2C bus - asynchronous, timeout occurs
 */
TEST_F(i2cDriverFixture, i2c_async_read_timeout_test)
{
   uint8_t test_address = 0xFF;
   EXPECT_CALL(*logger_mock, logger_get_group_state(_)).WillOnce(Return(LOGGER_GROUP_ENABLE));
   /**
    * <b>scenario</b>:  Async read from I2C bus - timeout occurs <br>
    * <b>expected</b>:  RETURN_OK returned, transaction started, callback with error called <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(1);
   EXPECT_EQ(RETURN_OK, i2c_read_async(test_address, 3, &fake_callback));

   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_START) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_ACK) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR2_ITBUFEN) != 0);

   simulate_SB_interrupt();
   EXPECT_EQ(I2C1->DR, test_address);

   simulate_ADDR_interrupt();

   i2c_on_timeout();

   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_, _, _)).Times(2);
   EXPECT_CALL(*callMock, callback(I2C_OP_READ,I2C_STATUS_ERROR,_,_));
   i2c_watcher();


   /**
    * <b>scenario</b>:  Async read from I2C bus - reading after timeout <br>
    * <b>expected</b>:  RETURN_OK returned, transaction started <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(1);
   EXPECT_EQ(RETURN_OK, i2c_read_async(test_address, 3, &fake_callback));

   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_START) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_ACK) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR2_ITBUFEN) != 0);

   simulate_SB_interrupt();
   EXPECT_EQ(I2C1->DR, test_address);

   simulate_ADDR_interrupt();

   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).Times(1);
   for (uint8_t i = 0; i < 3; i++)
   {
      simulate_RXNE_interrupt(i);
   }
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_STOP) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR2_ITBUFEN) == 0);

   EXPECT_CALL(*callMock, callback(I2C_OP_READ,I2C_STATUS_OK,_,3)).WillOnce(Invoke([&]
   (I2C_OP_TYPE, I2C_STATUS, const uint8_t* data, uint8_t)
   {
      EXPECT_EQ(data[0], 0x00);
      EXPECT_EQ(data[1], 0x01);
      EXPECT_EQ(data[2], 0x02);
   }));
   i2c_watcher();

}

/**
 * @test Setting/getting i2c timeout
 */
TEST_F(i2cDriverFixture, i2c_set_get_timeout_test)
{
   /**
    * <b>scenario</b>:  Setting too low timeout <br>
    * <b>expected</b>:  Timeout not changed <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, i2c_set_timeout(I2C_MIN_TIMEOUT_MS-1));

   /**
    * <b>scenario</b>:  Setting too high timeout <br>
    * <b>expected</b>:  Timeout not changed <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, i2c_set_timeout(I2C_MAX_TIMEOUT_MS+1));

   /**
    * <b>scenario</b>:  Setting correct timeout <br>
    * <b>expected</b>:  Timeout changed, scheduler notified <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_period(_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_OK, i2c_set_timeout(I2C_DEFAULT_TIMEOUT_MS+1));
   EXPECT_EQ(I2C_DEFAULT_TIMEOUT_MS+1, i2c_get_timeout());
}

/**
 * @test I2C driver is busy, checking behavior in case of requesting another operation
 */
TEST_F(i2cDriverFixture, i2c_busy_starting_read_test)
{
   uint8_t test_address = 0xFF;
   uint8_t test_buffer[2] = {0x00, 0x00};
   /**
    * <b>scenario</b>:  Send two read requests <br>
    * <b>expected</b>:  Only first request should be processed <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).Times(1);
   EXPECT_EQ(RETURN_OK, i2c_read_async(test_address, 3, &fake_callback));

   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_START) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR1_ACK) != 0);
   EXPECT_TRUE( (I2C1->CR1 & I2C_CR2_ITBUFEN) != 0);

   EXPECT_EQ(RETURN_NOK, i2c_read_async(test_address, 3, &fake_callback));
   EXPECT_EQ(RETURN_NOK, i2c_write_async(test_address, test_buffer, 2, &fake_callback));
}
