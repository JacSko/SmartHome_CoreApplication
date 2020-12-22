#ifndef _DHT_DRIVER_MOCK_H_
#define _DHT_DRIVER_MOCK_H_

#include "i2c_driver.h"
#include "gmock/gmock.h"

struct i2cDrvMock
{
   MOCK_METHOD0(i2c_initialize, RET_CODE());
   MOCK_METHOD4(i2c_write_async, RET_CODE(I2C_ADDRESS, const uint8_t*, uint8_t, I2C_CALLBACK));
   MOCK_METHOD3(i2c_write, I2C_STATUS(I2C_ADDRESS, const uint8_t*, uint8_t));
   MOCK_METHOD3(i2c_read_async, RET_CODE(I2C_ADDRESS, uint8_t, I2C_CALLBACK));
   MOCK_METHOD3(i2c_read, I2C_STATUS(I2C_ADDRESS, uint8_t*, uint8_t));
   MOCK_METHOD0(i2c_get_timeout, uint16_t());
   MOCK_METHOD1(i2c_set_timeout, RET_CODE(uint16_t));
   MOCK_METHOD0(i2c_watcher, void());
   MOCK_METHOD0(i2c_reset, void());
   MOCK_METHOD0(i2c_deinitialize, void());
};

i2cDrvMock* i2c_mock;

void mock_i2c_init()
{
   i2c_mock = new i2cDrvMock;
}

void mock_i2c_deinit()
{
	delete i2c_mock;
}

RET_CODE i2c_initialize()
{
   return i2c_mock->i2c_initialize();
}
RET_CODE i2c_write_async(I2C_ADDRESS address, const uint8_t* data, uint8_t size, I2C_CALLBACK callback)
{
   return i2c_mock->i2c_write_async(address, data, size, callback);
}
I2C_STATUS i2c_write(I2C_ADDRESS address, const uint8_t* data, uint8_t size)
{
   return i2c_mock->i2c_write(address, data, size);
}
RET_CODE i2c_read_async(I2C_ADDRESS address, uint8_t size, I2C_CALLBACK callback)
{
   return i2c_mock->i2c_read_async(address, size, callback);
}
I2C_STATUS i2c_read(I2C_ADDRESS address, uint8_t* data, uint8_t size)
{
   return i2c_mock->i2c_read(address, data, size);
}
uint16_t i2c_get_timeout()
{
   return i2c_mock->i2c_get_timeout();
}
RET_CODE i2c_set_timeout(uint16_t timeout)
{
   return i2c_mock->i2c_set_timeout(timeout);
}
void i2c_watcher()
{
   i2c_mock->i2c_watcher();
}
void i2c_reset()
{
   i2c_mock->i2c_reset();
}
void i2c_deinitialize()
{
   i2c_mock->i2c_deinitialize();
}

#endif
