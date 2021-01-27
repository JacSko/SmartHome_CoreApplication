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
#define ENV_ERROR_RATE_SAMPLES 100
/* =============================
 *       Internal types
 * =============================*/
typedef struct ENV_ERR_STAT
{
   uint8_t sample_id;
   DHT_STATUS err_map[ENV_ERROR_RATE_SAMPLES];
   ENV_ERROR_RATE err_rates;
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
   ENV_SENSOR sensors [ENV_SENSORS_COUNT];
   ENV_SENSOR* selected_sensor;
   uint16_t measure_period;
} ENV_MODULE;
/* =============================
 *   Internal module functions
 * =============================*/
void env_on_timeout();
void env_get_next_sensor(void);
void env_on_dht_data(DHT_STATUS, DHT_SENSOR*);
DHT_SENSOR_ID env_id_to_dht_id(ENV_ITEM_ID id);
void env_notify_listeners(ENV_EVENT event, ENV_ITEM_ID id,  const DHT_SENSOR*);
RET_CODE env_verify_measurement_period(uint16_t period);
void env_register_response(DHT_STATUS status);
void env_update_stats();
/* =============================
 *      Module variables
 * =============================*/
ENV_CLB_ITEM ENV_CLB_ITEMS[ENV_LISTENERS_MAX];
ENV_MODULE env_module;


RET_CODE env_initialize(const ENV_CONFIG* cfg)
{
   RET_CODE result = RETURN_NOK;

   if (cfg)
   {
      env_module.cfg = *cfg;
      logger_send(LOG_ENV, __func__, "got config: %u %u %u", env_module.cfg.measure_running, env_module.cfg.max_nr_rate, env_module.cfg.max_cs_rate);
      env_module.measure_period = ENV_MEASURE_PERIOD_DEF_MS;
      for (uint8_t i = 0; i < ENV_SENSORS_COUNT; i++)
      {
         env_module.sensors[i].dht_id = cfg->items[i].dht_id;
         env_module.sensors[i].env_id = cfg->items[i].env_id;
         env_module.sensors[i].type = DHT_TYPE_DHT11;
         env_module.sensors[i].data.temp_h = 0;
         env_module.sensors[i].data.temp_l = 0;
         env_module.sensors[i].data.hum_h = 0;
         env_module.sensors[i].data.hum_l = 0;
         env_module.sensors[i].stats.sample_id = 0;
         env_module.sensors[i].stats.err_rates.cs_err_rate = 0;
         env_module.sensors[i].stats.err_rates.nr_err_rate = 0;
         for (uint8_t i = 0; i < ENV_ERROR_RATE_SAMPLES; i++)
         {
            env_module.sensors[i].stats.err_map[i] = DHT_STATUS_OK;
         }
      }

      env_get_next_sensor();

      result = sch_subscribe_and_set(&env_on_timeout, TASKPRIO_LOW, env_module.measure_period,
               env_module.cfg.measure_running? TASKSTATE_RUNNING : TASKSTATE_STOPPED, TASKTYPE_PERIODIC);
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "ENV init error");
   return result;
}
void env_deinitialize()
{
   for (uint8_t i = 0; i < ENV_LISTENERS_MAX; i++)
   {
      ENV_CLB_ITEMS[i].callback = NULL;
      ENV_CLB_ITEMS[i].id = ENV_UNKNOWN_ITEM;
   }
   env_module.selected_sensor = NULL;

}
void env_on_timeout()
{
   logger_send(LOG_ENV, __func__, "ENV timeout");
   if (dht_read_async(env_module.selected_sensor->dht_id, &env_on_dht_data) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "cannot read sensor %u", env_module.selected_sensor->env_id);
   }
}
void env_on_dht_data(DHT_STATUS status, DHT_SENSOR* sensor)
{
   logger_send(LOG_ENV, __func__, "got dht data");
   ENV_EVENT event = ENV_EV_ERROR;
   if (status == DHT_STATUS_OK)
   {
      env_module.selected_sensor->data = sensor->data;
      if (env_module.selected_sensor->type != sensor->type)
      {
         logger_send(LOG_ERROR, __func__, "new sensor type, s:%u, t:%u", env_module.selected_sensor->dht_id, sensor->type);
      }
      env_module.selected_sensor->type = sensor->type;
      event = ENV_EV_NEW_DATA;
   }
   env_register_response(status);
   env_notify_listeners(event, env_module.selected_sensor->env_id, sensor);
   env_get_next_sensor();
}

void env_register_response(DHT_STATUS status)
{
   switch (status)
   {
   case DHT_STATUS_OK:
      break;
   case DHT_STATUS_NO_RESPONSE:
      logger_send(LOG_ERROR, __func__, "no response from %u", env_module.selected_sensor->env_id);
      break;
   case DHT_STATUS_CHECKSUM_ERROR:
      logger_send(LOG_ERROR, __func__, "cs error from %u", env_module.selected_sensor->env_id);
      break;
   default:
      break;
   }
   env_module.selected_sensor->stats.err_map[env_module.selected_sensor->stats.sample_id] = status;
   env_module.selected_sensor->stats.sample_id++;
   if (env_module.selected_sensor->stats.sample_id >= ENV_ERROR_RATE_SAMPLES)
   {
      env_module.selected_sensor->stats.sample_id = 0;
   }

   env_update_stats();
}
void env_update_stats()
{
   uint8_t nr_count = 0;
   uint8_t cs_count = 0;
   for (uint8_t i = 0; i < ENV_ERROR_RATE_SAMPLES; i++)
   {
      if (env_module.selected_sensor->stats.err_map[i] == DHT_STATUS_NO_RESPONSE)
      {
         nr_count++;
      }
      if (env_module.selected_sensor->stats.err_map[i] == DHT_STATUS_CHECKSUM_ERROR)
      {
         cs_count++;
      }
   }
   env_module.selected_sensor->stats.err_rates.cs_err_rate = (cs_count*ENV_ERROR_RATE_SAMPLES)/100;
   env_module.selected_sensor->stats.err_rates.nr_err_rate = (nr_count*ENV_ERROR_RATE_SAMPLES)/100;

}
void env_get_next_sensor()
{
   if (!env_module.selected_sensor || env_module.selected_sensor == &env_module.sensors[ENV_SENSORS_COUNT - 1])
   {
      env_module.selected_sensor = &env_module.sensors[0];
   }
   else
   {
      env_module.selected_sensor++;
   }
   logger_send(LOG_ENV, __func__, "new sensor selected %u", env_module.selected_sensor->env_id);
}
void env_notify_listeners(ENV_EVENT event, ENV_ITEM_ID id,  const DHT_SENSOR* sensor)
{
   for (uint8_t i = 0; i < ENV_LISTENERS_MAX; i++)
   {
      if (ENV_CLB_ITEMS[i].callback && ENV_CLB_ITEMS[i].id == id)
      {
         ENV_CLB_ITEMS[i].callback(event, id, sensor);
      }
   }
}
RET_CODE env_read_sensor(ENV_ITEM_ID id, DHT_SENSOR* buffer)
{
   RET_CODE result = RETURN_ERROR;
   if (buffer)
   {
      DHT_SENSOR sensor_data = {};
      DHT_STATUS status = dht_read(env_id_to_dht_id(id), &sensor_data);
      if (status == DHT_STATUS_OK)
      {
         *buffer = sensor_data;
         result = RETURN_OK;
      }
      else
      {
         result = RETURN_NOK;
      }
   }
   return result;
}
RET_CODE env_get_sensor_data(ENV_ITEM_ID id, DHT_SENSOR* buffer)
{
   RET_CODE result = RETURN_ERROR;
   if (buffer)
   {
      for (uint8_t i = 0; i < ENV_SENSORS_COUNT; i++)
      {
         if (env_module.sensors[i].env_id == id)
         {
            buffer->id = env_module.sensors[i].dht_id;
            buffer->type = env_module.sensors[i].type;
            buffer->data = env_module.sensors[i].data;
            result = RETURN_OK;
            break;
         }
      }
   }
   return result;
}
ENV_ERROR_RATE env_get_error_stats(ENV_ITEM_ID id)
{
   ENV_ERROR_RATE result;
   for (uint8_t i = 0; i < ENV_SENSORS_COUNT; i++)
   {
      if (env_module.sensors[i].env_id == id)
      {
         result = env_module.sensors[i].stats.err_rates;
      }
   }
   return result;
}
DHT_SENSOR_ID env_id_to_dht_id(ENV_ITEM_ID id)
{
   DHT_SENSOR_ID result = DHT_ENUM_MAX;
   for (uint8_t i = 0; i < ENV_SENSORS_COUNT; i++)
   {
      if (env_module.sensors[i].env_id == id)
      {
         result = env_module.sensors[i].dht_id;
         break;
      }
   }
   return result;
}
RET_CODE env_set_measurement_period(uint16_t period)
{
   RET_CODE result = RETURN_NOK;
   if (env_verify_measurement_period(period) == RETURN_OK)
   {
      result = sch_set_task_period(&env_on_timeout, period);
      if (result == RETURN_OK)
      {
         env_module.measure_period = period;
      }
   }
   return result;
}
uint16_t env_get_measurement_period()
{
   return env_module.measure_period;
}
RET_CODE env_verify_measurement_period(uint16_t period)
{
   RET_CODE result = RETURN_NOK;
   if (period >= ENV_MEASURE_PERIOD_MIN_MS && period <=ENV_MEASURE_PERIOD_MAX_MS)
   {
      result = RETURN_OK;
   }
   return result;
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

