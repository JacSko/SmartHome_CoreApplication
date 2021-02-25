#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/dht_driver_sim.c"
#include "socket_driver_mock.h"
#ifdef __cplusplus
}
#endif

#include "logger_mock.h"

/* ============================= */
/**
 * @file dht_driver_sim_tests.cpp
 *
 * @brief Unit tests of DHT driver module with software emulated hardware.
 *
 * @author Jacek Skowronek
 * @date 26/02/2021
 */
/* ============================= */


using namespace ::testing;

struct callbackMock
{
   MOCK_METHOD2(callback, void(DHT_STATUS, DHT_SENSOR*));
};

callbackMock* callMock;

void fake_callback(DHT_STATUS status, DHT_SENSOR* sensor)
{
   callMock->callback(status, sensor);
}

struct dhtdriverSimFixture : public ::testing::Test
{
   virtual void SetUp()
   {
      mock_logger_init();
      mock_sockdrv_init();

      EXPECT_CALL(*sockdrv_mock, sockdrv_create(_,_)).WillOnce(Return(control_if_fd))
                                                     .WillOnce(Return(app_id_fd));
      EXPECT_CALL(*sockdrv_mock, sockdrv_add_listener(_,_)).Times(2);
      hwstub_init();
      callMock = new callbackMock();
      dht_initialize();
   }

   virtual void TearDown()
   {
      EXPECT_CALL(*sockdrv_mock, sockdrv_close(_)).Times(2);
      EXPECT_CALL(*sockdrv_mock, sockdrv_remove_listener(_)).Times(2);
      hwstub_deinit();
      mock_sockdrv_deinit();
      mock_logger_deinit();
      delete callMock;
   }

   uint8_t control_if_fd = 1;
   uint8_t app_id_fd = 2;

};

/**
 * @test Reading DHT data from HW stub
 */
TEST_F(dhtdriverSimFixture, reading_data)
{
   DHT_SENSOR test_sensor, test_sensor2 = {};
   /**
    * <b>scenario</b>: Reading data to NULL buffer.<br>
    * <b>expected</b>: DHT_STATUS_ERROR returned.<br>
    * ************************************************
    */
   EXPECT_EQ(DHT_STATUS_ERROR, dht_read(DHT_SENSOR1, nullptr));

   /**
    * <b>scenario</b>: Reading data with invalid sensorID.<br>
    * <b>expected</b>: DHT_STATUS_ERROR returned.<br>
    * ************************************************
    */
   EXPECT_EQ(DHT_STATUS_ERROR, dht_read(DHT_ENUM_MAX, &test_sensor));

   /**
    * <b>scenario</b>: Sensor data set from test interface.<br>
    * <b>expected</b>: Correct data readout.<br>
    * ************************************************
    */
   hwstub_on_new_command(SOCK_DRV_NEW_DATA, "03 06 02 00 22 05 60 30");
   hwstub_watcher();
   hwstub_on_new_command(SOCK_DRV_NEW_DATA, "03 06 01 00 30 10 50 50");
   hwstub_watcher();

   EXPECT_EQ(DHT_STATUS_OK, dht_read(DHT_SENSOR3, &test_sensor));
   EXPECT_EQ(test_sensor.type, DHT_TYPE_DHT11);
   EXPECT_EQ(test_sensor.data.temp_h, 22);
   EXPECT_EQ(test_sensor.data.temp_l, 05);
   EXPECT_EQ(test_sensor.data.hum_h, 60);
   EXPECT_EQ(test_sensor.data.hum_l, 30);

   EXPECT_EQ(DHT_STATUS_OK, dht_read(DHT_SENSOR2, &test_sensor2));
   EXPECT_EQ(test_sensor2.type, DHT_TYPE_DHT11);
   EXPECT_EQ(test_sensor2.data.temp_h, 30);
   EXPECT_EQ(test_sensor2.data.temp_l, 10);
   EXPECT_EQ(test_sensor2.data.hum_h, 50);
   EXPECT_EQ(test_sensor2.data.hum_l, 50);

   /**
    * <b>scenario</b>: Read DHT sensor async - invalid ID.<br>
    * <b>expected</b>: Callback not called, ERROR status returned.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, dht_read_async(DHT_ENUM_MAX, &fake_callback));
   dht_data_watcher();

   /**
    * <b>scenario</b>: Read DHT sensor async - correct ID.<br>
    * <b>expected</b>: Callback called with correct data.<br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_OK, dht_read_async(DHT_SENSOR3, &fake_callback));
   EXPECT_CALL(*callMock, callback(_,_)).WillOnce(Invoke([&](DHT_STATUS status, DHT_SENSOR* sensor)
         {
            EXPECT_EQ(status, DHT_STATUS_OK);
            EXPECT_EQ(sensor->type, DHT_TYPE_DHT11);
            EXPECT_EQ(sensor->data.temp_h, 22);
            EXPECT_EQ(sensor->data.temp_l, 05);
            EXPECT_EQ(sensor->data.hum_h, 60);
            EXPECT_EQ(sensor->data.hum_l, 30);
         }));
   dht_data_watcher();

}

/**
 * @test Setting/Getting transaction timeout
 */
TEST_F(dhtdriverSimFixture, set_get_timeout_tests)
{
   /**
    * <b>scenario</b>: Timeout lower than possible <br>
    * <b>expected</b>: Timeout not changed <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, dht_set_timeout(DHT_MINIMUM_TIMEOUT_MS - 1));
   EXPECT_EQ(DHT_DEFAULT_TIMEOUT_MS, dht_get_timeout());

   /**
    * <b>scenario</b>: Timeout higher than possible <br>
    * <b>expected</b>: Timeout not changed <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_NOK, dht_set_timeout(DHT_MAXIMUM_TIMEOUT_MS + 1));
   EXPECT_EQ(DHT_DEFAULT_TIMEOUT_MS, dht_get_timeout());

   /**
    * <b>scenario</b>: Timeout in valid range <br>
    * <b>expected</b>: Timeout changed <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_OK, dht_set_timeout(DHT_DEFAULT_TIMEOUT_MS + 10));
   EXPECT_EQ(DHT_DEFAULT_TIMEOUT_MS + 10, dht_get_timeout());
}
