#include "gtest/gtest.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/dht_driver.c"
#include "../../../../ext_lib/CMSIS/stubs/device/stm32f4xx.h"
#ifdef __cplusplus
}
#endif

#include "logger_mock.h"
#include "gpio_lib_mock.h"
#include "task_scheduler_mock.h"

/* ============================= */
/**
 * @file dht_driver_tests.cpp
 *
 * @brief Unit tests of DHTXX Driver module
 *
 * @details
 * This tests verifies behavior of DHTXX Driver module
 *
 * @author Jacek Skowronek
 * @date 15/12/2020
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

struct dhtDriverFixture : public ::testing::Test
{
	virtual void SetUp()
	{
	   mock_logger_init();
		stm_stub_init();
		mock_gpio_init();
		mock_sch_init();
		callMock = new callbackMock();
	}

	virtual void TearDown()
	{
	   mock_logger_deinit();
		stm_stub_deinit();
		mock_gpio_deinit();
		mock_sch_deinit();
		delete callMock;
	}
};

/**
 * @test Reading data from one device - correct sequence
 */
TEST_F(dhtDriverFixture, reading_one_sensor_test)
{

	/**
	 * <b>scenario</b>: Request to read data from one sensor <br>
	 * <b>expected</b>: Data read and parsed correctly, callback called <br>
	 * ************************************************
	 */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_,_,_,_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(6);
   EXPECT_EQ(RETURN_OK, dht_initialize());

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));

   EXPECT_EQ(RETURN_OK, dht_read_async(DHT_SENSOR1, &fake_callback));

   //gpio should be set to low
   EXPECT_TRUE( (GPIOB->ODR & GPIO_ODR_ODR_9) == 0);

}









