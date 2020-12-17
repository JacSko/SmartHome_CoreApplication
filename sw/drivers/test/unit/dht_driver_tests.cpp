#include "gtest/gtest.h"
#include <thread>
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
		EXTI->PR = 0;
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

   std::vector<uint16_t> sensor_data = {9999, 9999,
                                        8000, 8000, 8000, 8000, 8000, 8000, 12000, 8000,
                                        8000, 12000, 8000, 12000, 12000, 8000, 8000, 8000,
                                        8000, 8000, 8000, 8000, 8000, 8000, 8000, 12000,
                                        8000, 8000, 12000, 8000, 12000, 12000, 8000, 8000,
                                        12000, 8000, 8000, 8000, 8000, 12000, 12000, 12000};
	/**
	 * <b>scenario</b>: Request to read data from SENSOR1 <br>
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
   EXPECT_FALSE( (GPIOB->ODR & GPIO_ODR_ODR_9) != 0);

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_DEFAULT_TIMEOUT_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   dht_on_timeout();

   EXPECT_TRUE( (GPIOB->ODR & GPIO_ODR_ODR_9) != 0);
   EXPECT_TRUE( (TIM2->CR1 & TIM_CR1_CEN) != 0);

   /* watchdog task should be disabled */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).WillOnce(Return(RETURN_OK));
   for (uint8_t i = 0; i < DHT_TIMESTAMPS_BUFFER_SIZE; i++)
   {
      EXTI->PR |= EXTI_PR_PR9;
      TIM2->CNT = sensor_data[i];
      EXTI9_5_IRQHandler();
      EXTI->PR = 0;
   }
   /* timer disabled, measurement flag set */
   EXPECT_FALSE( (TIM2->CR1 & TIM_CR1_CEN) != 0);
   EXPECT_TRUE(dht_measurement_ready);

   EXPECT_CALL(*callMock, callback(_,_)).WillOnce(Invoke([&](DHT_STATUS status, DHT_SENSOR* sensor)
         {
            EXPECT_EQ(status, DHT_STATUS_OK);
            EXPECT_EQ(sensor->id, DHT_SENSOR1);
            EXPECT_EQ(sensor->type, DHT_TYPE_DHT22);
            EXPECT_EQ(sensor->data.temp_h, 30);
            EXPECT_EQ(sensor->data.temp_l, 0);
            EXPECT_EQ(sensor->data.hum_h, 60);
            EXPECT_EQ(sensor->data.hum_l, 0);
         }));
   dht_data_watcher();
   EXPECT_FALSE(dht_measurement_ready);


   /**
    * <b>scenario</b>: Second read sequence for SENSOR2 <br>
    * <b>expected</b>: Data read and parsed correctly, callback called <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_OK, dht_read_async(DHT_SENSOR2, &fake_callback));

   //gpio should be set to low
   EXPECT_FALSE( (GPIOB->ODR & GPIO_ODR_ODR_10) != 0);

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_DEFAULT_TIMEOUT_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   dht_on_timeout();

   EXPECT_TRUE( (GPIOB->ODR & GPIO_ODR_ODR_10) != 0);
   EXPECT_TRUE( (TIM2->CR1 & TIM_CR1_CEN) != 0);

   /* watchdog task should be disabled */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).WillOnce(Return(RETURN_OK));
   for (uint8_t i = 0; i < DHT_TIMESTAMPS_BUFFER_SIZE; i++)
   {
      EXTI->PR |= EXTI_PR_PR10;
      TIM2->CNT = sensor_data[i];
      EXTI15_10_IRQHandler();
      EXTI->PR = 0;
   }
   /* timer disabled, measurement flag set */
   EXPECT_FALSE( (TIM2->CR1 & TIM_CR1_CEN) != 0);
   EXPECT_TRUE(dht_measurement_ready);

   EXPECT_CALL(*callMock, callback(_,_)).WillOnce(Invoke([&](DHT_STATUS status, DHT_SENSOR* sensor)
         {
            EXPECT_EQ(status, DHT_STATUS_OK);
            EXPECT_EQ(sensor->id, DHT_SENSOR2);
            EXPECT_EQ(sensor->type, DHT_TYPE_DHT22);
            EXPECT_EQ(sensor->data.temp_h, 30);
            EXPECT_EQ(sensor->data.temp_l, 0);
            EXPECT_EQ(sensor->data.hum_h, 60);
            EXPECT_EQ(sensor->data.hum_l, 0);
         }));
   dht_data_watcher();
   EXPECT_FALSE(dht_measurement_ready);
}

/**
 * @test Reading data from one device - checksum error
 */
TEST_F(dhtDriverFixture, reading_one_sensor_checksum_error_test)
{

   std::vector<uint16_t> sensor_data_checsum_error = {9999, 9999,
                                        8000, 8000, 8000, 8000, 8000, 8000, 12000, 8000,
                                        8000, 12000, 8000, 12000, 12000, 8000, 8000, 8000,
                                        8000, 8000, 8000, 8000, 8000, 8000, 8000, 12000,
                                        8000, 8000, 12000, 8000, 12000, 12000, 8000, 8000,
                                        12000, 8000, 12000, 8000, 8000, 12000, 12000, 12000};
   std::vector<uint16_t> sensor_data = {9999, 9999,
                                        8000, 8000, 8000, 8000, 8000, 8000, 12000, 8000,
                                        8000, 12000, 8000, 12000, 12000, 8000, 8000, 8000,
                                        8000, 8000, 8000, 8000, 8000, 8000, 8000, 12000,
                                        8000, 8000, 12000, 8000, 12000, 12000, 8000, 8000,
                                        12000, 8000, 8000, 8000, 8000, 12000, 12000, 12000};
   /**
    * <b>scenario</b>: Request to read data from SENSOR3 <br>
    * <b>expected</b>: Checksum error catched, callback called with error status <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_,_,_,_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(6);
   EXPECT_EQ(RETURN_OK, dht_initialize());

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));

   EXPECT_EQ(RETURN_OK, dht_read_async(DHT_SENSOR3, &fake_callback));

   //gpio should be set to low
   EXPECT_FALSE( (GPIOB->ODR & GPIO_ODR_ODR_12) != 0);

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_DEFAULT_TIMEOUT_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   dht_on_timeout();

   EXPECT_TRUE( (GPIOB->ODR & GPIO_ODR_ODR_12) != 0);
   EXPECT_TRUE( (TIM2->CR1 & TIM_CR1_CEN) != 0);

   /* watchdog task should be disabled */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).WillOnce(Return(RETURN_OK));
   for (uint8_t i = 0; i < DHT_TIMESTAMPS_BUFFER_SIZE; i++)
   {
      EXTI->PR |= EXTI_PR_PR12;
      TIM2->CNT = sensor_data_checsum_error[i];
      EXTI15_10_IRQHandler();
      EXTI->PR = 0;
   }
   /* timer disabled, measurement flag set */
   EXPECT_FALSE( (TIM2->CR1 & TIM_CR1_CEN) != 0);
   EXPECT_TRUE(dht_measurement_ready);

   EXPECT_CALL(*callMock, callback(_,_)).WillOnce(Invoke([&](DHT_STATUS status, DHT_SENSOR* sensor)
         {
            EXPECT_EQ(status, DHT_STATUS_CHECKSUM_ERROR);
            EXPECT_EQ(sensor->id, DHT_SENSOR3);
         }));
   dht_data_watcher();
   EXPECT_FALSE(dht_measurement_ready);

   /**
    * <b>scenario</b>: Second read sequence for SENSOR4 <br>
    * <b>expected</b>: Data read and parsed correctly, callback called <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_OK, dht_read_async(DHT_SENSOR4, &fake_callback));

   //gpio should be set to low
   EXPECT_FALSE( (GPIOB->ODR & GPIO_ODR_ODR_13) != 0);

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_DEFAULT_TIMEOUT_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   dht_on_timeout();

   EXPECT_TRUE( (GPIOB->ODR & GPIO_ODR_ODR_13) != 0);
   EXPECT_TRUE( (TIM2->CR1 & TIM_CR1_CEN) != 0);

   /* watchdog task should be disabled */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).WillOnce(Return(RETURN_OK));
   for (uint8_t i = 0; i < DHT_TIMESTAMPS_BUFFER_SIZE; i++)
   {
      EXTI->PR |= EXTI_PR_PR13;
      TIM2->CNT = sensor_data[i];
      EXTI15_10_IRQHandler();
      EXTI->PR = 0;
   }
   /* timer disabled, measurement flag set */
   EXPECT_FALSE( (TIM2->CR1 & TIM_CR1_CEN) != 0);
   EXPECT_TRUE(dht_measurement_ready);

   EXPECT_CALL(*callMock, callback(_,_)).WillOnce(Invoke([&](DHT_STATUS status, DHT_SENSOR* sensor)
         {
            EXPECT_EQ(status, DHT_STATUS_OK);
            EXPECT_EQ(sensor->id, DHT_SENSOR4);
            EXPECT_EQ(sensor->type, DHT_TYPE_DHT22);
            EXPECT_EQ(sensor->data.temp_h, 30);
            EXPECT_EQ(sensor->data.temp_l, 0);
            EXPECT_EQ(sensor->data.hum_h, 60);
            EXPECT_EQ(sensor->data.hum_l, 0);
         }));
   dht_data_watcher();
   EXPECT_FALSE(dht_measurement_ready);
}

/**
 * @test Reading data from one device - no response
 */
TEST_F(dhtDriverFixture, reading_one_sensor_no_response)
{
   std::vector<uint16_t> sensor_data = {9999, 9999,
                                        8000, 8000, 8000, 8000, 8000, 8000, 12000, 8000,
                                        8000, 12000, 8000, 12000, 12000, 8000, 8000, 8000,
                                        8000, 8000, 8000, 8000, 8000, 8000, 8000, 12000,
                                        8000, 8000, 12000, 8000, 12000, 12000, 8000, 8000,
                                        12000, 8000, 8000, 8000, 8000, 12000, 12000, 12000};
   /**
    * <b>scenario</b>: Request to read data for SENSOR5 <br>
    * <b>expected</b>: Data read and parsed correctly, callback called <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_,_,_,_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(6);
   EXPECT_EQ(RETURN_OK, dht_initialize());

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));

   EXPECT_EQ(RETURN_OK, dht_read_async(DHT_SENSOR5, &fake_callback));

   //gpio should be set to low
   EXPECT_FALSE( (GPIOB->ODR & GPIO_ODR_ODR_14) != 0);

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_DEFAULT_TIMEOUT_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   dht_on_timeout();

   EXPECT_TRUE( (GPIOB->ODR & GPIO_ODR_ODR_14) != 0);
   EXPECT_TRUE( (TIM2->CR1 & TIM_CR1_CEN) != 0);

   /* simulate timeout*/
   dht_on_timeout();

   /* timer disabled, measurement flag set */
   EXPECT_FALSE( (TIM2->CR1 & TIM_CR1_CEN) != 0);
   EXPECT_TRUE(dht_measurement_ready);

   EXPECT_CALL(*callMock, callback(_,_)).WillOnce(Invoke([&](DHT_STATUS status, DHT_SENSOR* sensor)
         {
            EXPECT_EQ(status, DHT_STATUS_NO_RESPONSE);
            EXPECT_EQ(sensor->id, DHT_SENSOR5);
            EXPECT_EQ(sensor->type, DHT_TYPE_DHT22);
            EXPECT_EQ(sensor->data.temp_h, 30);
            EXPECT_EQ(sensor->data.temp_l, 0);
            EXPECT_EQ(sensor->data.hum_h, 60);
            EXPECT_EQ(sensor->data.hum_l, 0);
         }));
   dht_data_watcher();
   EXPECT_FALSE(dht_measurement_ready);

   /**
    * <b>scenario</b>: Second read sequence for SENSOR6 <br>
    * <b>expected</b>: Data read and parsed correctly, callback called <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_OK, dht_read_async(DHT_SENSOR6, &fake_callback));

   //gpio should be set to low
   EXPECT_FALSE( (GPIOB->ODR & GPIO_ODR_ODR_15) != 0);

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_DEFAULT_TIMEOUT_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   dht_on_timeout();

   EXPECT_TRUE( (GPIOB->ODR & GPIO_ODR_ODR_15) != 0);
   EXPECT_TRUE( (TIM2->CR1 & TIM_CR1_CEN) != 0);
   /* watchdog task should be disabled */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).WillOnce(Return(RETURN_OK));
   for (uint8_t i = 0; i < DHT_TIMESTAMPS_BUFFER_SIZE; i++)
   {

      EXTI->PR |= EXTI_PR_PR15;
      TIM2->CNT = sensor_data[i];
      EXTI15_10_IRQHandler();
      EXTI->PR = 0;
   }
   /* timer disabled, measurement flag set */
   EXPECT_FALSE( (TIM2->CR1 & TIM_CR1_CEN) != 0);
   EXPECT_TRUE(dht_measurement_ready);

   EXPECT_CALL(*callMock, callback(_,_)).WillOnce(Invoke([&](DHT_STATUS status, DHT_SENSOR* sensor)
         {
            EXPECT_EQ(status, DHT_STATUS_OK);
            EXPECT_EQ(sensor->id, DHT_SENSOR6);
            EXPECT_EQ(sensor->type, DHT_TYPE_DHT22);
            EXPECT_EQ(sensor->data.temp_h, 30);
            EXPECT_EQ(sensor->data.temp_l, 0);
            EXPECT_EQ(sensor->data.hum_h, 60);
            EXPECT_EQ(sensor->data.hum_l, 0);
         }));
   dht_data_watcher();
   EXPECT_FALSE(dht_measurement_ready);
}

/**
 * @test Reading data from DHT11 device
 */
TEST_F(dhtDriverFixture, reading_dht11_type_sensor)
{
   std::vector<uint16_t> sensor_data = {9999, 9999,
                                        8000, 8000, 12000, 12000, 12000, 12000, 8000, 8000,
                                        8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000,
                                        8000, 8000, 8000, 12000, 12000, 12000, 12000, 8000,
                                        8000, 8000, 8000, 8000, 8000, 8000, 12000, 8000,
                                        8000, 12000, 8000, 12000, 12000, 12000, 8000, 8000};
   /**
    * <b>scenario</b>: Request to read data for SENSOR5 - type DHT11 <br>
    * <b>expected</b>: Data read and parsed correctly, callback called <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_,_,_,_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(6);
   EXPECT_EQ(RETURN_OK, dht_initialize());

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));

   EXPECT_EQ(RETURN_OK, dht_read_async(DHT_SENSOR5, &fake_callback));

   //gpio should be set to low
   EXPECT_FALSE( (GPIOB->ODR & GPIO_ODR_ODR_14) != 0);

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_DEFAULT_TIMEOUT_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   dht_on_timeout();

   EXPECT_TRUE( (GPIOB->ODR & GPIO_ODR_ODR_14) != 0);
   EXPECT_TRUE( (TIM2->CR1 & TIM_CR1_CEN) != 0);
   /* watchdog task should be disabled */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).WillOnce(Return(RETURN_OK));
   for (uint8_t i = 0; i < DHT_TIMESTAMPS_BUFFER_SIZE; i++)
   {

      EXTI->PR |= EXTI_PR_PR14;
      TIM2->CNT = sensor_data[i];
      EXTI15_10_IRQHandler();
      EXTI->PR = 0;
   }
   /* timer disabled, measurement flag set */
   EXPECT_FALSE( (TIM2->CR1 & TIM_CR1_CEN) != 0);
   EXPECT_TRUE(dht_measurement_ready);

   EXPECT_CALL(*callMock, callback(_,_)).WillOnce(Invoke([&](DHT_STATUS status, DHT_SENSOR* sensor)
         {
            EXPECT_EQ(status, DHT_STATUS_OK);
            EXPECT_EQ(sensor->id, DHT_SENSOR5);
            EXPECT_EQ(sensor->type, DHT_TYPE_DHT11);
            EXPECT_EQ(sensor->data.temp_h, 30);
            EXPECT_EQ(sensor->data.temp_l, 2);
            EXPECT_EQ(sensor->data.hum_h, 60);
            EXPECT_EQ(sensor->data.hum_l, 0);
         }));
   dht_data_watcher();
   EXPECT_FALSE(dht_measurement_ready);

   /**
    * <b>scenario</b>: Second read sequence for SENSOR6 <br>
    * <b>expected</b>: Data read and parsed correctly, callback called <br>
    * ************************************************
    */
   std::vector<uint16_t> sensor_data_neg_temp = {9999, 9999,
                                        8000, 8000, 8000, 8000, 8000, 8000, 12000, 8000,
                                        8000, 12000, 8000, 12000, 12000, 8000, 8000, 8000,
                                        12000, 8000, 8000, 8000, 8000, 8000, 8000, 12000,
                                        8000, 8000, 12000, 8000, 12000, 12000, 8000, 8000,
                                        8000, 8000, 8000, 8000, 8000, 12000, 12000, 12000};
   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_OK, dht_read_async(DHT_SENSOR6, &fake_callback));

   //gpio should be set to low
   EXPECT_FALSE( (GPIOB->ODR & GPIO_ODR_ODR_15) != 0);

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_DEFAULT_TIMEOUT_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   dht_on_timeout();

   EXPECT_TRUE( (GPIOB->ODR & GPIO_ODR_ODR_15) != 0);
   EXPECT_TRUE( (TIM2->CR1 & TIM_CR1_CEN) != 0);
   /* watchdog task should be disabled */
   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).WillOnce(Return(RETURN_OK));
   for (uint8_t i = 0; i < DHT_TIMESTAMPS_BUFFER_SIZE; i++)
   {

      EXTI->PR |= EXTI_PR_PR15;
      TIM2->CNT = sensor_data_neg_temp[i];
      EXTI15_10_IRQHandler();
      EXTI->PR = 0;
   }
   /* timer disabled, measurement flag set */
   EXPECT_FALSE( (TIM2->CR1 & TIM_CR1_CEN) != 0);
   EXPECT_TRUE(dht_measurement_ready);

   EXPECT_CALL(*callMock, callback(_,_)).WillOnce(Invoke([&](DHT_STATUS status, DHT_SENSOR* sensor)
         {
            EXPECT_EQ(status, DHT_STATUS_OK);
            EXPECT_EQ(sensor->id, DHT_SENSOR6);
            EXPECT_EQ(sensor->type, DHT_TYPE_DHT22);
            EXPECT_EQ(sensor->data.temp_h, -30);
            EXPECT_EQ(sensor->data.temp_l, 0);
            EXPECT_EQ(sensor->data.hum_h, 60);
            EXPECT_EQ(sensor->data.hum_l, 0);
         }));
   dht_data_watcher();
   EXPECT_FALSE(dht_measurement_ready);
}

/**
 * @test Reading data from one device - no response
 */
TEST_F(dhtDriverFixture, set_get_timeout_tests)
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

/**
 * @test Reading start - negative cases
 */
TEST_F(dhtDriverFixture, read_async_negative_cases)
{
   /**
    * <b>scenario</b>: Request read for invalid sensor <br>
    * <b>expected</b>: Measurement not started <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_,_,_,_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(6);
   EXPECT_EQ(RETURN_OK, dht_initialize());

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_NOK, dht_read_async(DHT_ENUM_MAX, &fake_callback));

   /**
    * <b>scenario</b>: Driver is busy, but another request arrives <br>
    * <b>expected</b>: Measurement not started <br>
    * ************************************************
    */
   EXPECT_EQ(RETURN_OK, dht_read_async(DHT_SENSOR6, &fake_callback));
   EXPECT_EQ(RETURN_NOK, dht_read_async(DHT_SENSOR6, &fake_callback));

   /**
    * <b>scenario</b>: Cannot set period of timer for start sequence <br>
    * <b>expected</b>: Status ERROR returned <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_,_,_,_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(6);
   EXPECT_EQ(RETURN_OK, dht_initialize());

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_NOK));
   EXPECT_EQ(RETURN_NOK, dht_read_async(DHT_SENSOR6, &fake_callback));

   /**
    * <b>scenario</b>: Cannot run timer for start sequence <br>
    * <b>expected</b>: Status ERROR returned <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_,_,_,_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(6);
   EXPECT_EQ(RETURN_OK, dht_initialize());

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_NOK));
   EXPECT_EQ(RETURN_NOK, dht_read_async(DHT_SENSOR6, &fake_callback));

   /**
    * <b>scenario</b>: Cannot set period of timer for watchdog <br>
    * <b>expected</b>: Callback called with error status <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_,_,_,_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(6);
   EXPECT_EQ(RETURN_OK, dht_initialize());

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_OK, dht_read_async(DHT_SENSOR6, &fake_callback));

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_DEFAULT_TIMEOUT_MS)).WillOnce(Return(RETURN_NOK));
   dht_on_timeout();
   EXPECT_TRUE(dht_measurement_ready);

   EXPECT_CALL(*callMock, callback(_,_)).WillOnce(Invoke([&](DHT_STATUS status, DHT_SENSOR* sensor)
         {
            EXPECT_EQ(status, DHT_STATUS_ERROR);
            EXPECT_EQ(sensor->id, DHT_SENSOR6);
         }));
   dht_data_watcher();
   EXPECT_FALSE(dht_measurement_ready);

   /**
    * <b>scenario</b>: Cannot run timer for start sequence <br>
    * <b>expected</b>: Status ERROR returned <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_,_,_,_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(6);
   EXPECT_EQ(RETURN_OK, dht_initialize());

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_NOK));
   EXPECT_EQ(RETURN_NOK, dht_read_async(DHT_SENSOR6, &fake_callback));

   /**
    * <b>scenario</b>: Cannot start timer for watchdog <br>
    * <b>expected</b>: Callback called with error status <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_,_,_,_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(6);
   EXPECT_EQ(RETURN_OK, dht_initialize());

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK));
   EXPECT_EQ(RETURN_OK, dht_read_async(DHT_SENSOR6, &fake_callback));

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_DEFAULT_TIMEOUT_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_NOK));
   dht_on_timeout();
   EXPECT_TRUE(dht_measurement_ready);

   EXPECT_CALL(*callMock, callback(_,_)).WillOnce(Invoke([&](DHT_STATUS status, DHT_SENSOR* sensor)
         {
            EXPECT_EQ(status, DHT_STATUS_ERROR);
            EXPECT_EQ(sensor->id, DHT_SENSOR6);
         }));
   dht_data_watcher();
   EXPECT_FALSE(dht_measurement_ready);
}



/**
 * @test Reading data from sensor in block way
 */
TEST_F(dhtDriverFixture, read_blocking_test)
{
   std::vector<uint16_t> sensor_data = {9999, 9999,
                                        8000, 8000, 8000, 8000, 8000, 8000, 12000, 8000,
                                        8000, 12000, 8000, 12000, 12000, 8000, 8000, 8000,
                                        8000, 8000, 8000, 8000, 8000, 8000, 8000, 12000,
                                        8000, 8000, 12000, 8000, 12000, 12000, 8000, 8000,
                                        12000, 8000, 8000, 8000, 8000, 12000, 12000, 12000};
   /**
    * <b>scenario</b>: Request blocking data read from SENSOR1 <br>
    * <b>expected</b>: Data read and parsed correctly, callback called <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_,_,_,_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(6);
   EXPECT_EQ(RETURN_OK, dht_initialize());

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_DEFAULT_TIMEOUT_MS)).WillOnce(Return(RETURN_OK));
   dht_on_timeout();

   EXPECT_CALL(*sch_mock, sch_set_task_state(_, TASKSTATE_STOPPED)).WillOnce(Return(RETURN_OK));

   /* thread have to be started because of while loop in dht_read */
   std::thread thread([&]()
         {
            while(dht_driver.state != DHT_STATE_START);
            dht_on_timeout();
            while(dht_driver.state != DHT_STATE_READING);
            for (uint8_t i = 0; i < DHT_TIMESTAMPS_BUFFER_SIZE; i++)
            {
               EXTI->PR |= EXTI_PR_PR9;
               TIM2->CNT = sensor_data[i];
               EXTI9_5_IRQHandler();
               EXTI->PR = 0;
            }
         });

   DHT_SENSOR sensor = {};
   EXPECT_EQ(DHT_STATUS_OK, dht_read(DHT_SENSOR1, &sensor));
   EXPECT_EQ(DHT_TYPE_DHT22, sensor.type);
   EXPECT_EQ(DHT_SENSOR1, sensor.id);
   EXPECT_EQ(60, sensor.data.hum_h);
   EXPECT_EQ(0, sensor.data.hum_l);
   EXPECT_EQ(30, sensor.data.temp_h);
   EXPECT_EQ(0, sensor.data.temp_l);

   thread.join();
}

/**
 * @test Reading data from sensor in block way - no response from device
 */
TEST_F(dhtDriverFixture, read_blocking_test_timeout)
{
   std::vector<uint16_t> sensor_data = {9999, 9999,
                                        8000, 8000, 8000, 8000, 8000, 8000, 12000, 8000,
                                        8000, 12000, 8000, 12000, 12000, 8000, 8000, 8000,
                                        8000, 8000, 8000, 8000, 8000, 8000, 8000, 12000,
                                        8000, 8000, 12000, 8000, 12000, 12000, 8000, 8000,
                                        12000, 8000, 8000, 8000, 8000, 12000, 12000, 12000};
   /**
    * <b>scenario</b>: Request blocking data read from SENSOR1 - no reponse from device <br>
    * <b>expected</b>: Function returns after timeout - no endless loop <br>
    * ************************************************
    */
   EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_,_,_,_,_)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*gpio_lib_mock, gpio_pin_cfg(_,_,_)).Times(6);
   EXPECT_EQ(RETURN_OK, dht_initialize());

   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_START_TIME_MS)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_trigger_task(_)).WillOnce(Return(RETURN_OK)).WillOnce(Return(RETURN_OK));
   EXPECT_CALL(*sch_mock, sch_set_task_period(_,DHT_DEFAULT_TIMEOUT_MS)).WillOnce(Return(RETURN_OK));
   dht_on_timeout();

   /* thread have to be started because of while loop in dht_read */
   std::thread thread([&]()
         {
            while(dht_driver.state != DHT_STATE_START);
            dht_on_timeout();
            while(dht_driver.state != DHT_STATE_READING);
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            dht_on_timeout();
         });

   DHT_SENSOR sensor = {};
   EXPECT_EQ(DHT_STATUS_NO_RESPONSE, dht_read(DHT_SENSOR1, &sensor));
   EXPECT_EQ(DHT_SENSOR1, sensor.id);

   thread.join();
}
