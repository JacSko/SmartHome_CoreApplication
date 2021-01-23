/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdint.h>
#include <stdlib.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "bathroom_fan.h"
#include "relays_board.h"
#include "env_monitor.h"
#include "Logger.h"
#include "task_scheduler.h"
/* =============================
 *          Defines
 * =============================*/
#define FAN_MAX_LISTENERS 5
#define FAN_MIN_WORKING_TIME_S 10 /* 10s */
#define FAN_MAX_WORKING_TIME_S 7200 /* 7200s - 2h */
#define FAN_MAX_HUMIDITY_THR 95
#define FAN_MIN_HUMIDITY_THR 20
#define FAN_MAX_HUM_HYSTER 20
#define FAN_MIN_HUM_HYSTER 0
/* =============================
 *       Internal types
 * =============================*/
typedef enum FAN_TRIGGER_TYPE
{
   FAN_TRIGGER_AUTO,
   FAN_TRIGGER_MANUAL,
} FAN_TRIGGER_TYPE;
typedef struct FAN_MODULE
{
   FAN_STATE state;
   FAN_TRIGGER_TYPE trigger;
   uint16_t working_time;
   FAN_CONFIG cfg;
} FAN_MODULE;
/* =============================
 *   Internal module functions
 * =============================*/
void fan_on_timeout();
void fan_on_env_data(ENV_EVENT event, ENV_ITEM_ID id,  const DHT_SENSOR*);
RET_CODE fan_validate_working_time(uint16_t time_s);
RET_CODE fan_start_with_type(FAN_TRIGGER_TYPE type);
void fan_notify_change();
RET_CODE fan_stop_state(FAN_STATE state);
/* =============================
 *      Module variables
 * =============================*/
FAN_MODULE fan_module;
void (*FAN_CALLBACKS[FAN_MAX_LISTENERS])(FAN_STATE);

RET_CODE fan_initialize(const FAN_CONFIG* config)
{
   RET_CODE result = RETURN_NOK;
   logger_send(LOG_FAN, __func__, "");
   fan_module.state = FAN_STATE_UNKNOWN;
   if (config)
   {
      fan_module.cfg = *config;
      if (env_register_listener(&fan_on_env_data, ENV_BATHROOM) == RETURN_OK)
      {
         result = sch_subscribe_and_set(&fan_on_timeout, TASKPRIO_LOW, 1000, TASKSTATE_STOPPED, TASKTYPE_PERIODIC);
         fan_stop_state(FAN_STATE_OFF);
         logger_send(LOG_FAN, __func__, "success");
      }
   }

   for (uint8_t i = 0; i < FAN_MAX_LISTENERS; i++)
   {
      FAN_CALLBACKS[i] = NULL;
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR,__func__, "error");
   return result;
}
void fan_deinitialize()
{
   sch_unsubscribe(&fan_on_timeout);
   env_unregister_listener(&fan_on_env_data, ENV_BATHROOM);
   for (uint8_t i = 0; i < FAN_MAX_LISTENERS; i++)
   {
      FAN_CALLBACKS[i] = NULL;
   }
}

void fan_on_env_data(ENV_EVENT event, ENV_ITEM_ID id, const DHT_SENSOR* sensor)
{
   logger_send(LOG_FAN, __func__, "e %u id %u temp %u.%.2u hum %u.%.2u", event, id, sensor->data.temp_h, sensor->data.temp_l,
                                                                     sensor->data.hum_h, sensor->data.hum_l);
   if (event == ENV_EV_NEW_DATA && id == ENV_BATHROOM && fan_module.trigger == FAN_TRIGGER_AUTO)
   {
      switch(fan_module.state)
      {
      case FAN_STATE_OFF:
         if (sensor->data.hum_h >= fan_module.cfg.fan_humidity_threshold)
         {
            fan_start_with_type(FAN_TRIGGER_AUTO);
         }
         break;
      case FAN_STATE_ON:
         if (sensor->data.hum_h <= (fan_module.cfg.fan_humidity_threshold - fan_module.cfg.fan_threshold_hysteresis)
             && fan_module.working_time >= fan_module.cfg.min_working_time_s)
         {
            fan_stop_state(FAN_STATE_OFF);
         }
         break;
      case FAN_STATE_SUSPEND:
         if (sensor->data.hum_h <= fan_module.cfg.fan_humidity_threshold - fan_module.cfg.fan_threshold_hysteresis)
         {
            fan_stop_state(FAN_STATE_OFF);
            logger_send(LOG_FAN, __func__, "return from suspend");
         }
         break;
      default:
         logger_send(LOG_FAN, __func__, "unhandled state");
         break;
      }
   }
   else
   {
      logger_send(LOG_ERROR, __func__, "incorrect data ev %d id %d", event, id);
   }
}
void fan_on_timeout()
{
   if (fan_module.state == FAN_STATE_ON)
   {
      fan_module.working_time++;
      if (fan_module.working_time >= fan_module.cfg.max_working_time_s)
      {
         fan_stop_state(FAN_STATE_SUSPEND);
         logger_send(LOG_ERROR, __func__, "max working time reached, suspended");
      }
   }
   else
   {
      logger_send(LOG_ERROR, __func__, "sch event when state different than ON, stopping task");
      sch_set_task_state(&fan_on_timeout, TASKSTATE_STOPPED);
   }

}

RET_CODE fan_start_with_type(FAN_TRIGGER_TYPE type)
{
   RET_CODE result = RETURN_NOK;

   if (fan_module.state != FAN_STATE_ON)
   {
      switch (type)
      {
      case FAN_TRIGGER_AUTO:
         fan_module.trigger = FAN_TRIGGER_AUTO;
         fan_module.working_time = 0;
         result = sch_set_task_state(&fan_on_timeout, TASKSTATE_RUNNING);
         break;
      case FAN_TRIGGER_MANUAL:
         fan_module.trigger = FAN_TRIGGER_MANUAL;
         result = RETURN_OK;
         break;
      }
   }

   if (result == RETURN_OK)
   {
      result = rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_ON);
      if (result == RETURN_OK)
      {
         fan_module.state = FAN_STATE_ON;
         fan_notify_change();
         logger_send(LOG_FAN, __func__, "starting fan, trigger %u", type);
      }
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "t: %u, s: %u", type, fan_module.state);
   return result;
}
RET_CODE fan_add_listener(FAN_LISTENER listener)
{
   RET_CODE result = RETURN_NOK;
   for (uint8_t i = 0; i < FAN_MAX_LISTENERS; i++)
   {
      if (FAN_CALLBACKS[i] == NULL)
      {
         FAN_CALLBACKS[i] = listener;
         result = RETURN_OK;
         break;
      }
   }
   return result;
}
RET_CODE fan_remove_listener(FAN_LISTENER listener)
{
   RET_CODE result = RETURN_NOK;
   for (uint8_t i = 0; i < FAN_MAX_LISTENERS; i++)
   {
      if (FAN_CALLBACKS[i] == listener)
      {
         FAN_CALLBACKS[i] = NULL;
         result = RETURN_OK;
         break;
      }
   }
   return result;
}
void fan_notify_change()
{
   for (uint8_t i = 0; i < FAN_MAX_LISTENERS; i++)
   {
      if (FAN_CALLBACKS[i])
      {
         FAN_CALLBACKS[i](fan_module.state);
      }
   }
}
RET_CODE fan_start()
{
   return fan_start_with_type(FAN_TRIGGER_MANUAL);
}
RET_CODE fan_stop()
{
   RET_CODE result = RETURN_NOK;
   if (fan_module.state == FAN_STATE_ON)
   {
      result = fan_stop_state(FAN_STATE_OFF);
   }
   return result;
}
RET_CODE fan_stop_state(FAN_STATE state)
{
   RET_CODE result = RETURN_OK;
   logger_send(LOG_FAN, __func__, "new state %u", state);
   if (fan_module.state != FAN_STATE_SUSPEND)
   {
      sch_set_task_state(&fan_on_timeout, TASKSTATE_STOPPED);
      fan_module.trigger = FAN_TRIGGER_AUTO;
      result = rel_set(RELAY_BATHROOM_FAN, RELAY_STATE_OFF);
   }
   fan_module.state = state;
   fan_notify_change();
   fan_module.working_time = 0;

   return result;
}
FAN_STATE fan_get_state()
{
   return fan_module.state;
}
RET_CODE fan_set_max_working_time(uint16_t time_s)
{
   RET_CODE result = RETURN_NOK;
   if (fan_validate_working_time(time_s) == RETURN_OK)
   {
      fan_module.cfg.max_working_time_s = time_s;
      result = RETURN_OK;
      logger_send(LOG_FAN, __func__, "new time %u", time_s);
   }
   return result;
}
RET_CODE fan_set_min_working_time(uint16_t time_s)
{
   RET_CODE result = RETURN_NOK;
   if (fan_validate_working_time(time_s) == RETURN_OK)
   {
      fan_module.cfg.min_working_time_s = time_s;
      result = RETURN_OK;
      logger_send(LOG_FAN, __func__, "new time %u", time_s);
   }
   return result;
}
RET_CODE fan_validate_working_time(uint16_t time_s)
{
   RET_CODE result = RETURN_NOK;
   if (time_s >= FAN_MIN_WORKING_TIME_S && time_s <= FAN_MAX_WORKING_TIME_S)
   {
      result = RETURN_OK;
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR,  __func__, "invalid time %u", time_s);
   return result;
}
RET_CODE fan_set_humidity_threshold(uint8_t hum_trigger)
{
   RET_CODE result = RETURN_NOK;
   if (hum_trigger >= FAN_MIN_HUMIDITY_THR && hum_trigger <= FAN_MAX_HUMIDITY_THR)
   {
      fan_module.cfg.fan_humidity_threshold = hum_trigger;
      result = RETURN_OK;
      logger_send(LOG_FAN, __func__, "new thr %u", hum_trigger);
   }
   return result;
}
RET_CODE fan_set_threshold_hysteresis(uint8_t hysteresis)
{
   RET_CODE result = RETURN_NOK;
   if (hysteresis >= FAN_MIN_HUM_HYSTER && hysteresis <= FAN_MAX_HUM_HYSTER)
   {
      fan_module.cfg.fan_threshold_hysteresis = hysteresis;
      result = RETURN_OK;
      logger_send(LOG_FAN, __func__, "new hysteresis %u", hysteresis);
   }
   return result;
}
RET_CODE fan_get_config(FAN_CONFIG* buffer)
{
   RET_CODE result = RETURN_NOK;
   if (buffer)
   {
      *buffer = fan_module.cfg;
      result = RETURN_OK;
   }
   return result;
}
