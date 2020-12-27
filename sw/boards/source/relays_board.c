/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdlib.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "relays_board.h"
#include "task_scheduler.h"
#include "Logger.h"
/* =============================
 *          Defines
 * =============================*/
#define RELAYS_VERIFICATION_STATE 1
#define RELAYS_VERIFICATION_DEF_TIME_MS 5000
#define RELAYS_VERIFICATION_MIN_TIME_MS 100
#define RELAYS_VERIFICATION_MAX_TIME_MS 30000
/* =============================
 *   Internal module functions
 * =============================*/
uint8_t rel_id_to_relay_no(RELAY_ID id);
uint16_t rel_id_to_mask(RELAY_ID id);
uint16_t rel_relay_to_mask(uint8_t relay_no);
RET_CODE rel_verify_period(uint16_t period);
void rel_on_new_data(I2C_OP_TYPE type, I2C_STATUS status, const uint8_t* data, uint8_t size);
void rel_on_timeout();
/* =============================
 *       Internal types
 * =============================*/
typedef struct RELAY_MODULE
{
   RELAYS_CONFIG cfg;
   uint16_t current_relays;
   uint8_t verification_enabled;
   uint16_t verification_time;
} RELAY_MODULE;
/* =============================
 *      Module variables
 * =============================*/
RELAY_MODULE rel_module;


RET_CODE rel_initialize(const RELAYS_CONFIG* config)
{
   RET_CODE result = RETURN_NOK;
   if (config)
   {
      logger_send(LOG_RELAYS, __func__, "");
      rel_module.verification_enabled = RELAYS_VERIFICATION_STATE;
      rel_module.verification_time = RELAYS_VERIFICATION_DEF_TIME_MS;
      rel_module.current_relays = 0x0000;
      rel_module.cfg = *config;
      result = sch_subscribe_and_set(&rel_on_timeout, TASKPRIO_LOW, rel_module.verification_time,
               rel_module.verification_enabled? TASKSTATE_RUNNING : TASKSTATE_STOPPED, TASKTYPE_PERIODIC);
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "error during initialization");
   return result;
}
void rel_deinitialize()
{
   sch_unsubscribe(&rel_on_timeout);
}

uint8_t rel_id_to_relay_no(RELAY_ID id)
{
   uint8_t result = 0;
   for (uint8_t i = 0; i < RELAYS_BOARD_COUNT; i++)
   {
      if (rel_module.cfg.items[i].id == id)
      {
         result = rel_module.cfg.items[i].relay_no;
         break;
      }
   }
   return result;
}

uint16_t rel_relay_to_mask(uint8_t relay_no)
{
   return (uint16_t)(1 << (relay_no - 1));
}

uint16_t rel_id_to_mask(RELAY_ID id)
{
   return rel_relay_to_mask(rel_id_to_relay_no(id));
}

RET_CODE rel_set(RELAY_ID id, RELAY_STATE state)
{
   RET_CODE result = RETURN_NOK;
   if (id < RELAY_ID_ENUM_MAX && state < RELAY_STATE_ENUM_MAX)
   {
      uint16_t new_relays = (uint16_t) ~(rel_module.current_relays ^ rel_id_to_mask(id));
      I2C_STATUS ret = i2c_write(rel_module.cfg.address, (uint8_t*)&new_relays, 2);
      if (ret == I2C_STATUS_OK)
      {
         result = RETURN_OK;
         rel_module.current_relays = ~new_relays;
         logger_send(LOG_RELAYS, __func__, "new state %x", rel_module.current_relays);
      }
      else
      {
         logger_send(LOG_ERROR, __func__, "cannot sent data to relays");
      }

   }
   return result;
}
RELAY_STATE rel_get(RELAY_ID id)
{
   RELAY_STATE result = RELAY_STATE_ENUM_MAX;
   uint16_t mask = rel_id_to_mask(id);
   if (mask)
   {
      result = (rel_module.current_relays & mask) > 0? RELAY_STATE_ON : RELAY_STATE_OFF;
   }
   return result;
}
RET_CODE rel_get_all(RELAY_STATUS* buffer)
{
   RET_CODE result = RETURN_NOK;
   if (buffer)
   {
      for (uint8_t i = 0; i < RELAYS_BOARD_COUNT; i++)
      {
         buffer[i].id = rel_module.cfg.items[i].id;
         buffer[i].state = rel_get(buffer[i].id);
      }
      result = RETURN_OK;
   }
   return result;
}
RET_CODE rel_get_config(RELAYS_CONFIG* buffer)
{
   RET_CODE result = RETURN_NOK;
   if (buffer)
   {
      *buffer = rel_module.cfg;
      result = RETURN_OK;
   }
   return result;
}
RET_CODE rel_set_verification_period(uint16_t period)
{
   RET_CODE result = RETURN_NOK;
   if (rel_verify_period(period) == RETURN_OK)
   {
      if (sch_set_task_period(&rel_on_timeout, period) == RETURN_OK)
      {
         logger_send(LOG_RELAYS, __func__, "new verification period: %d", period);
         result = RETURN_OK;
         rel_module.verification_time = period;
      }
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "cannot set verification period; %d", period);
   return result;
}
RET_CODE rel_verify_period(uint16_t period)
{
   RET_CODE result = (period >= RELAYS_VERIFICATION_MIN_TIME_MS && period <= RELAYS_VERIFICATION_MAX_TIME_MS)? RETURN_OK : RETURN_NOK;
   return result;
}
uint16_t rel_get_verification_period()
{
   return rel_module.verification_time;
}
void rel_enable_verification()
{
   sch_set_task_state(&rel_on_timeout, TASKSTATE_RUNNING);
   rel_module.verification_enabled = 1;
   logger_send(LOG_RELAYS, __func__, "enabling relays autoupdate");
}
void rel_disable_verification()
{
   sch_set_task_state(&rel_on_timeout, TASKSTATE_STOPPED);
   rel_module.verification_enabled = 0;
   logger_send(LOG_RELAYS, __func__, "disabling relays autoupdate");
}
RET_CODE rel_get_verification_state()
{
   RET_CODE result = rel_module.verification_enabled? RETURN_OK : RETURN_NOK;
   return result;
}
void rel_on_timeout()
{
   logger_send(LOG_RELAYS, __func__, "relays timeout - reading");
   if (i2c_read_async(rel_module.cfg.address + 1, 2, &rel_on_new_data) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "cannot read relays");
   }
}
void rel_on_new_data(I2C_OP_TYPE type, I2C_STATUS status, const uint8_t* data, uint8_t size)
{
   if (type == I2C_OP_READ && status == I2C_STATUS_OK && size == 2)
   {
      logger_send(LOG_RELAYS, __func__, "new i2c data: %x %x", data[0], data[1]);
      uint16_t recv_relays = data[0];
      recv_relays |= (data[1] << 8);
      recv_relays = ~recv_relays;
      logger_send(LOG_RELAYS, __func__, "new relays read: %x", recv_relays);
      if (rel_module.current_relays != recv_relays)
      {
         logger_send(LOG_ERROR, __func__, "relay mismatch detected: old %x new %x", rel_module.current_relays, recv_relays);
         rel_module.current_relays = recv_relays;
      }
   }
   else
   {
      logger_send(LOG_ERROR, __func__, "i2c error t:%d st:%d, size:%d", type, status, size);
   }
}
