#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/i2c_driver_sim.c"
#include "socket_driver_mock.h"
#include "inputs_board_mock.h"
#ifdef __cplusplus
}
#endif

#include "logger_mock.h"

/* ============================= */
/**
 * @file i2c_driver_sim_tests.cpp
 *
 * @brief Unit tests of I2C driver module with software emulated hardware.
 *
 * @author Jacek Skowronek
 * @date 26/02/2021
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

struct i2cdriverSimFixture : public ::testing::Test
{
   virtual void SetUp()
   {
      mock_logger_init();
      mock_sockdrv_init();
      mock_inp_init();

      EXPECT_CALL(*sockdrv_mock, sockdrv_create(_,_)).WillOnce(Return(control_if_fd))
                                                     .WillOnce(Return(app_id_fd));
      EXPECT_CALL(*sockdrv_mock, sockdrv_add_listener(_,_)).Times(2);
      hwstub_init();
      callMock = new callbackMock();
      i2c_initialize();
   }

   virtual void TearDown()
   {
      i2c_deinitialize();
      EXPECT_CALL(*sockdrv_mock, sockdrv_close(_)).Times(2);
      EXPECT_CALL(*sockdrv_mock, sockdrv_remove_listener(_)).Times(2);
      hwstub_deinit();
      mock_sockdrv_deinit();
      mock_logger_deinit();
      mock_inp_deinit();
      delete callMock;
   }


   uint8_t control_if_fd = 1;
   uint8_t app_id_fd = 2;
};

/**
 * @test Reading I2C data from HW stub
 */
TEST_F(i2cdriverSimFixture, reading_data)
{
   uint8_t data_size = 2;
   uint8_t result_buffer [data_size] = {};
   I2C_ADDRESS correct_address = 0x40;
   I2C_ADDRESS incorrect_address = 0x00;

   /**
    * <b>scenario</b>: Reading data to NULL buffer.<br>
    * <b>expected</b>: I2C_STATUS_ERROR returned.<br>
    * ************************************************
    */
   EXPECT_EQ(I2C_STATUS_ERROR, i2c_read(correct_address, nullptr, data_size));

   /**
    * <b>scenario</b>: Reading data for existing device.<br>
    * <b>expected</b>: Correct data returned.<br>
    * ************************************************
    */
   EXPECT_EQ(I2C_STATUS_OK, i2c_read(correct_address, result_buffer, data_size));
   EXPECT_EQ(result_buffer[0], 0x00);
   EXPECT_EQ(result_buffer[1], 0x00);

   /**
    * <b>scenario</b>: Reading data for non existing device.<br>
    * <b>expected</b>: Correct data returned.<br>
    * ************************************************
    */
   EXPECT_EQ(I2C_STATUS_ERROR, i2c_read(incorrect_address, result_buffer, data_size));

   /**
    * <b>scenario</b>: Reading data async from not existing device.<br>
    * <b>expected</b>: Callback not called, RETURN_NOK returned.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, i2c_read_async(incorrect_address, data_size, &fake_callback));
   i2c_watcher();

   /**
    * <b>scenario</b>: Reading data async, no callback provided.<br>
    * <b>expected</b>: RETURN_OK returned.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_OK, i2c_read_async(correct_address, data_size, nullptr));
   i2c_watcher();

   /**
    * <b>scenario</b>: Reading data async from existing device.<br>
    * <b>expected</b>: Callback called, RETURN_OK returned.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_OK, i2c_read_async(correct_address, data_size, &fake_callback));
   EXPECT_CALL(*callMock, callback(I2C_OP_READ, I2C_STATUS_OK, _, 2)).WillOnce(Invoke([&]
     (I2C_OP_TYPE, I2C_STATUS, const uint8_t* data, uint8_t)
     {
         EXPECT_EQ(data[0], 0x00);
         EXPECT_EQ(data[1], 0x00);
     }));
   i2c_watcher();

}

/**
 * @test Writing I2C data to HW stub
 */
TEST_F(i2cdriverSimFixture, writing_data)
{
   uint8_t data_size = 2;
   uint8_t test_buffer [data_size] = {0x11, 0x22};
   uint8_t result_buffer [data_size] = {};
   I2C_ADDRESS correct_address = 0x40;
   I2C_ADDRESS incorrect_address = 0x00;

   /**
    * <b>scenario</b>: Writing data - null buffer provided.<br>
    * <b>expected</b>: I2C_STATUS_ERROR returned.<br>
    * ************************************************
    */
   EXPECT_EQ(I2C_STATUS_ERROR, i2c_write(correct_address, nullptr, data_size));

   /**
    * <b>scenario</b>: Writing data - not existing device.<br>
    * <b>expected</b>: I2C_STATUS_ERROR returned.<br>
    * ************************************************
    */
   EXPECT_EQ(I2C_STATUS_ERROR, i2c_write(incorrect_address, test_buffer, data_size));

   /**
    * <b>scenario</b>: Writing data to existing device.<br>
    * <b>expected</b>: I2C_STATUS_OK returned, notification sent to test framework.<br>
    * ************************************************
    */
   const char* exp = "02 03 64 17 34\n";
   EXPECT_CALL(*sockdrv_mock, sockdrv_write(control_if_fd,_, _)).WillOnce(Invoke([&](sock_id id, const char* data, size_t size) -> RET_CODE
   {
      EXPECT_STREQ((char*)data, exp);
      return RETURN_OK;
   }));
   EXPECT_EQ(I2C_STATUS_OK, i2c_write(correct_address, test_buffer, data_size));

   /**
    * <b>scenario</b>: Reading data previously written.<br>
    * <b>expected</b>: Correct data returned.<br>
    * ************************************************
    */
   EXPECT_EQ(I2C_STATUS_OK, i2c_read(correct_address, result_buffer, data_size));
   EXPECT_EQ(result_buffer[0], 0x11);
   EXPECT_EQ(result_buffer[1], 0x22);

   /**
    * <b>scenario</b>: Writing data async.<br>
    * <b>expected</b>: RETURN_OK returned, notification sent to test framework.<br>
    * ************************************************
    */
   test_buffer[0] = 0x12;
   test_buffer[1] = 0x23;
   result_buffer[0] = 0x00;
   result_buffer[1] = 0x00;

   const char* exp_write = "02 03 64 18 35\n";
   EXPECT_CALL(*sockdrv_mock, sockdrv_write(control_if_fd,_, _)).WillOnce(Invoke([&](sock_id id, const char* data, size_t size) -> RET_CODE
   {
      EXPECT_STREQ((char*)data, exp_write);
      return RETURN_OK;
   }));

   EXPECT_CALL(*callMock, callback(I2C_OP_WRITE, I2C_STATUS_OK, _, 2)).WillOnce(Invoke([&]
   (I2C_OP_TYPE, I2C_STATUS, const uint8_t* data, uint8_t)
   {
      EXPECT_EQ(data[0], 0x00);
      EXPECT_EQ(data[1], 0x00);
   }));

   EXPECT_EQ(RETURN_OK, i2c_write_async(correct_address, test_buffer, data_size, &fake_callback));
   i2c_watcher();
}

/**
 * @test Settings of I2C board state from test framework
 */
TEST_F(i2cdriverSimFixture, setting_i2c_device_state_from_test_framework)
{
   uint8_t data_size = 2;
   uint8_t result_buffer [data_size] = {};
   I2C_ADDRESS correct_address = 0x40;

   /**
    * <b>scenario</b>: Incorrect event received.<br>
    * <b>expected</b>: Board state not set.<br>
    * ************************************************
    */
   const char* i2c_state_ok = "01 03 64 255 255";

   hwstub_on_new_command(SOCK_DRV_DISCONNECTED, i2c_state_ok);
   hwstub_watcher();
   EXPECT_EQ(I2C_STATUS_OK, i2c_read(correct_address, result_buffer, data_size));
   EXPECT_EQ(result_buffer[0], 0x00);
   EXPECT_EQ(result_buffer[1], 0x00);

   /**
    * <b>scenario</b>: Incorrect message structure received.<br>
    * <b>expected</b>: Board state not set.<br>
    * ************************************************
    */
   i2c_state_ok = "01 02 64 255 255";

   hwstub_on_new_command(SOCK_DRV_NEW_DATA, i2c_state_ok);
   hwstub_watcher();
   EXPECT_EQ(I2C_STATUS_OK, i2c_read(correct_address, result_buffer, data_size));
   EXPECT_EQ(result_buffer[0], 0x00);
   EXPECT_EQ(result_buffer[1], 0x00);

   /**
    * <b>scenario</b>: I2C command with wrong address recevied.<br>
    * <b>expected</b>: Board state not set.<br>
    * ************************************************
    */
   i2c_state_ok = "01 03 00 255 255";

   hwstub_on_new_command(SOCK_DRV_NEW_DATA, i2c_state_ok);
   hwstub_watcher();
   EXPECT_EQ(I2C_STATUS_OK, i2c_read(correct_address, result_buffer, data_size));
   EXPECT_EQ(result_buffer[0], 0x00);
   EXPECT_EQ(result_buffer[1], 0x00);

   /**
    * <b>scenario</b>: Correct I2C message received.<br>
    * <b>expected</b>: Board state set correctly.<br>
    * ************************************************
    */
   i2c_state_ok = "01 03 64 255 255";

   hwstub_on_new_command(SOCK_DRV_NEW_DATA, i2c_state_ok);
   hwstub_watcher();
   EXPECT_EQ(I2C_STATUS_OK, i2c_read(correct_address, result_buffer, data_size));
   EXPECT_EQ(result_buffer[0], 0xFF);
   EXPECT_EQ(result_buffer[1], 0xFF);



}

/**
 * @test Setting/getting i2c timeout
 */
TEST_F(i2cdriverSimFixture, i2c_set_get_timeout_test)
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
   EXPECT_EQ(RETURN_OK, i2c_set_timeout(I2C_DEFAULT_TIMEOUT_MS+1));
   EXPECT_EQ(I2C_DEFAULT_TIMEOUT_MS+1, i2c_get_timeout());
}

/**
 * @test Test of simulating hardware interrupt
 */
TEST_F(i2cdriverSimFixture, i2c_interrupt_simulation_test)
{
   /**
    * <b>scenario</b>:  Interrupt trigger message received <br>
    * <b>expected</b>:  Inputs mock called <br>
    * ************************************************
    */
   const char* i2c_state_ok = "04 00";

   EXPECT_CALL(*inp_mock, inp_on_interrupt_recevied());
   hwstub_on_new_command(SOCK_DRV_NEW_DATA, i2c_state_ok);
   hwstub_watcher();
}
