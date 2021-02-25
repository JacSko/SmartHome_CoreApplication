/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdlib.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "dht_driver.h"
#include "Logger.h"
#include "hw_stub.h"
/* =============================
 *          Defines
 * =============================*/
#define DHT_MINIMUM_TIMEOUT_MS 30
#define DHT_DEFAULT_TIMEOUT_MS 100
#define DHT_MAXIMUM_TIMEOUT_MS 300
#define DHT_START_TIME_MS 20
#define DHT_TIM_VALUE_LEVEL 10000
#define DHT_TIMESTAMPS_BUFFER_SIZE 42
#define DHT_TEMP_NEGATIVE_MASK 0x8000
/* =============================
 *   Internal module functions
 * =============================*/
RET_CODE dht_verify_timeout(uint16_t);
/* =============================
 *       Internal types
 * =============================*/
typedef struct DHT_SENSOR_MASKS
{
   uint32_t gpio_odr_mask;
   uint32_t exti_pr_mask;
} DHT_SENSOR_MASKS;
typedef enum DHT_DRIVER_STATE
{
   DHT_STATE_IDLE,
   DHT_STATE_START,
   DHT_STATE_READING,
   DHT_STATE_DATA_DECODING,
   DHT_STATE_TIMEOUT,
   DHT_STATE_CHECKSUM_ERROR,
   DHT_STATE_ERROR,
} DHT_DRIVER_STATE;
typedef struct DHT_DRIVER
{
   uint16_t timeout;
   uint8_t raw_data[5];
   DHT_DRIVER_STATE state;
   DHT_SENSOR sensor;
} DHT_DRIVER;
/* =============================
 *      Module variables
 * =============================*/
uint32_t DHT_MEASURE_TIMESTAMPS[DHT_TIMESTAMPS_BUFFER_SIZE];
DHT_SENSOR_MASKS DHT_REG_MAP[DHT_ENUM_MAX];
volatile uint8_t dht_timestamp_idx;
volatile uint8_t dht_measurement_ready;
DHT_DRIVER dht_driver;
DHT_CALLBACK dht_callback;

RET_CODE dht_initialize()
{
   RET_CODE result = RETURN_NOK;

   logger_send(LOG_DHT_DRV, __func__, "start");
   dht_driver.state = DHT_STATE_IDLE;
   dht_driver.timeout = DHT_DEFAULT_TIMEOUT_MS;
   dht_measurement_ready = 0;
   result = RETURN_OK;
   logger_send(LOG_DHT_DRV, __func__, "end");
   return result;
}

RET_CODE dht_read_async(DHT_SENSOR_ID id, DHT_CALLBACK clb)
{
   DHT_STATUS result = RETURN_NOK;
   if (hwstub_dht_read(id, &dht_driver.sensor) == DHT_STATUS_OK)
   {
      result = RETURN_OK;
   }
   dht_callback = clb;
   dht_measurement_ready = 1;
   logger_send(LOG_DHT_DRV, __func__, "read sync status %d", result);
   return result;
}

DHT_STATUS dht_read(DHT_SENSOR_ID id, DHT_SENSOR *sensor)
{
   DHT_STATUS result = hwstub_dht_read(id, sensor);
   logger_send(LOG_DHT_DRV, __func__, "read async status %d", result);
   return result;
}

RET_CODE dht_set_timeout(uint16_t timeout)
{
   RET_CODE result = RETURN_NOK;
   if (dht_verify_timeout(timeout) == RETURN_OK)
   {
      dht_driver.timeout = timeout;
      logger_send(LOG_DHT_DRV, __func__, "new timeout %d", timeout);
   }
   return result;
}

RET_CODE dht_verify_timeout(uint16_t period)
{
   RET_CODE result = RETURN_NOK;
   if (period >= DHT_MINIMUM_TIMEOUT_MS && period <= DHT_MAXIMUM_TIMEOUT_MS)
   {
      result = RETURN_OK;
   }
   logger_send_if(result != RETURN_OK, LOG_DHT_DRV, __func__, "incorrect timeout %d", period);
   return result;
}

uint16_t dht_get_timeout()
{
   return dht_driver.timeout;
}

void dht_data_watcher()
{
   if (dht_measurement_ready == 1)
   {
      dht_measurement_ready = 0;
      if (dht_callback)
      {
         dht_callback(DHT_STATUS_OK, &dht_driver.sensor);
      }
      dht_driver.state = DHT_STATE_IDLE;
   }
}
