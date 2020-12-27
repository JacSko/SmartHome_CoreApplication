/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdlib.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "env_monitor.h"
#include "task_scheduler.h"
#include "Logger.h"
/* =============================
 *          Defines
 * =============================*/
#define ENV_LISTENERS_MAX 20
#define ENV_MEASURE_PERIOD_MIN_MS 1000
#define ENV_MEASURE_PERIOD_DEF_MS 5000
#define ENV_MEASURE_PERIOD_MAX_MS 30000
/* =============================
 *       Internal types
 * =============================*/
typedef struct ENV_ERR_STAT
{
   uint8_t nr_err_rate;
   uint8_t cs_err_rate;
} ENV_ERR_STAT;
typedef struct ENV_SENSOR
{
   ENV_ITEM_ID env_id;
   DHT_SENSOR_ID dht_id;
   DHT_SENSOR_TYPE type;
   DHT_SENSOR_DATA data;
   ENV_ERR_STAT stats;
} ENV_SENSOR;

typedef struct ENV_CLB_ITEM
{
   ENV_ITEM_ID id;
   ENV_CALLBACK callback;
} ENV_CLB_ITEM;

typedef struct ENV_MODULE
{
   ENV_CONFIG cfg;
   ENV_SENSOR sensors [ENV_ITEM_COUNT];
   ENV_SENSOR* selected_sensor;
   uint16_t measure_period;
} ENV_MODULE;
/* =============================
 *   Internal module functions
 * =============================*/
void env_on_timeout();
ENV_SENSOR* env_get_next_sensor(ENV_SENSOR* sensor);
void env_on_dht_data(DHT_STATUS, DHT_SENSOR*);
/* =============================
 *      Module variables
 * =============================*/
ENV_CLB_ITEM ENV_CLB_ITEMS[ENV_LISTENERS_MAX];

ENV_MODULE env_module;


RET_CODE env_initialize(const ENV_CONFIG* cfg)
{
   RET_CODE result = RETURN_NOK;
   for (uint8_t i = 0; i < ENV_LISTENERS_MAX; i++)
   {
      ENV_CLB_ITEMS[i].callback = NULL;
      ENV_CLB_ITEMS[i].id = ENV_UNKNOWN_ITEM;
   }

   if (cfg)
   {
      env_module.cfg = *cfg;
      env_module.measure_period = ENV_MEASURE_PERIOD_DEF_MS;
      for (uint8_t i = 0; i < ENV_ITEM_COUNT; i++)
      {
         env_module.sensors[i].dht_id = cfg->items[i].dht_id;
         env_module.sensors[i].env_id = cfg->items[i].env_id;
         env_module.sensors[i].type = DHT_TYPE_DHT11;
         env_module.sensors[i].data.temp_h = 0;
         env_module.sensors[i].data.temp_l = 0;
         env_module.sensors[i].data.hum_h = 0;
         env_module.sensors[i].data.hum_l = 0;
         env_module.sensors[i].stats.cs_err_rate = 0;
         env_module.sensors[i].stats.nr_err_rate = 0;
         env_module.selected_sensor = env_get_next_sensor(NULL);
      }

      if (sch_subscribe_and_set(&env_on_timeout, TASKPRIO_LOW, env_module.measure_period,
          env_module.cfg.measure_running? TASKSTATE_RUNNING : TASKSTATE_STOPPED, TASKTYPE_PERIODIC) == RETURN_OK)
      {
         result = RETURN_OK;
      }
   }
   return result;
}
void env_deinitialize()
{

}
void env_on_timeout()
{
   if (dht_read_async(env_module.selected_sensor->dht_id, &env_on_dht_data) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "cannot read sensor %u", env_module.selected_sensor->dht_id);
   }
}
void env_on_dht_data(DHT_STATUS status, DHT_SENSOR* sensor)
{
   switch (status)
   {
   case DHT_STATUS_OK:
      env_module.selected_sensor->data = sensor->data;
      if (env_module.selected_sensor->type != sensor->type)
      {
         logger_send(LOG_ERROR, __func__, "new sensor type, s:%u, t:%u", env_module.selected_sensor->dht_id, sensor->type);
      }
      env_module.selected_sensor->type = sensor->type;
      break;
   case DHT_STATUS_CHECKSUM_ERROR:
      //TODO handle error
      logger_send(LOG_ERROR, __func__, "Checksum error");
      break;
   case DHT_STATUS_NO_RESPONSE:
      //TODO handle error
      logger_send(LOG_ERROR, __func__, "no response");
      break;
   case DHT_STATUS_ERROR:
      //TODO handle error
      logger_send(LOG_ERROR, __func__, "generic error");
      break;
   }
   env_module.selected_sensor = env_get_next_sensor(env_module.selected_sensor);
}

ENV_SENSOR* env_get_next_sensor(ENV_SENSOR* sensor)
{
   ENV_SENSOR* result = sensor;
   if (!sensor || sensor == &env_module.sensors[ENV_ITEM_COUNT - 1])
   {
      result = &env_module.sensors[0];
   }
   else
   {
      result++;
   }
   return result;
}
RET_CODE env_read_sensor(ENV_ITEM_ID id, DHT_SENSOR_DATA* buffer)
{

}
RET_CODE env_set_measurement_period(uint16_t period)
{

}
uint16_t env_get_measurement_period()
{

}
RET_CODE env_register_listener(ENV_CALLBACK callback, ENV_ITEM_ID id)
{
   RET_CODE result = RETURN_NOK;
   for (uint8_t i = 0; i < ENV_LISTENERS_MAX; i++)
   {
      if (! ENV_CLB_ITEMS[i].callback)
      {
         ENV_CLB_ITEMS[i].id = id;
         ENV_CLB_ITEMS[i].callback = callback;
         result = RETURN_OK;
         break;
      }
      if ( (ENV_CLB_ITEMS[i].callback == callback) && (ENV_CLB_ITEMS[i].id == id))
      {
         result = RETURN_ERROR;
         break;
      }
   }
   return result;
}
void env_unregister_listener(ENV_CALLBACK callback, ENV_ITEM_ID id)
{
   for (uint8_t i = 0; i < ENV_LISTENERS_MAX; i++)
   {
      if ( (ENV_CLB_ITEMS[i].callback == callback) && (ENV_CLB_ITEMS[i].id == id))
      {
         ENV_CLB_ITEMS[i].callback = NULL;
         ENV_CLB_ITEMS[i].id = ENV_UNKNOWN_ITEM;
         break;
      }
   }
}

