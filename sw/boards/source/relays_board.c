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
#define RELAYS_CALLBACK_MAX_SIZE 5
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
void rel_read_state();
void rel_update_relays_state(uint16_t relays, RELAY_STATUS* status);
void rel_notify_change(const RELAY_STATUS* status);
/* =============================
 *       Internal types
 * =============================*/
typedef struct RELAY_MODULE
{
   RELAYS_CONFIG cfg;
   uint16_t current_relays;
   uint8_t verification_enabled;
   uint16_t verification_time;
   RELAY_STATUS current_status [RELAYS_BOARD_COUNT];
} RELAY_MODULE;
/* =============================
 *      Module variables
 * =============================*/
RELAY_MODULE rel_module;
void (*REL_CALLBACKS[RELAYS_CALLBACK_MAX_SIZE])(const RELAY_STATUS*);

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
      result = sch_subscribe_and_set(&rel_read_state, TASKPRIO_LOW, rel_module.verification_time,
               rel_module.verification_enabled? TASKSTATE_RUNNING : TASKSTATE_STOPPED, TASKTYPE_PERIODIC);
      for (uint8_t i = 0; i < RELAYS_CALLBACK_MAX_SIZE; i++)
      {
         REL_CALLBACKS[i] = NULL;
      }
      for (uint8_t i = 0; i < RELAYS_BOARD_COUNT; i++)
      {
         rel_module.current_status[i].id = rel_module.cfg.items[i].id;
         rel_module.current_status[i].state = RELAY_STATE_OFF;
      }

      rel_read_state();
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "error during initialization");
   return result;
}
void rel_deinitialize()
{
   sch_unsubscribe(&rel_read_state);
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
      uint16_t new_relays = ~rel_module.current_relays;
      if (state == RELAY_STATE_ON)
      {
         new_relays &= ~(rel_id_to_mask(id));
      }
      else
      {
         new_relays |= rel_id_to_mask(id);
      }
      I2C_STATUS ret = i2c_write(rel_module.cfg.address, (uint8_t*)&new_relays, 2);
      if (ret == I2C_STATUS_OK)
      {
         result = RETURN_OK;
         rel_module.current_relays = ~new_relays;
         rel_update_relays_state(rel_module.current_relays, rel_module.current_status);
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
   for (uint8_t i = 0; i < RELAYS_BOARD_COUNT; i++)
   {
      if (rel_module.current_status[i].id == id)
      {
         result = rel_module.current_status[i].state;
      }
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
         buffer[i] = rel_module.current_status[i];
      }
      result = RETURN_OK;
   }
   return result;
}
RET_CODE rel_read_all(RELAY_STATUS* buffer)
{
   RET_CODE result = RETURN_NOK;
   if (buffer)
   {
      uint8_t data [2];
      if (i2c_read(rel_module.cfg.address + 1, data, 2) == I2C_STATUS_OK)
      {
         uint16_t recv_relays = data[0];
         recv_relays |= (data[1] << 8);
         recv_relays = ~recv_relays;
         rel_update_relays_state(recv_relays, buffer);
         result = RETURN_OK;
      }
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
      if (sch_set_task_period(&rel_read_state, period) == RETURN_OK)
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
   sch_set_task_state(&rel_read_state, TASKSTATE_RUNNING);
   rel_module.verification_enabled = 1;
   logger_send(LOG_RELAYS, __func__, "enabling relays autoupdate");
}
void rel_disable_verification()
{
   sch_set_task_state(&rel_read_state, TASKSTATE_STOPPED);
   rel_module.verification_enabled = 0;
   logger_send(LOG_RELAYS, __func__, "disabling relays autoupdate");
}
RET_CODE rel_get_verification_state()
{
   RET_CODE result = rel_module.verification_enabled? RETURN_OK : RETURN_NOK;
   return result;
}
void rel_read_state()
{
   logger_send(LOG_RELAYS, __func__, "relays timeout - reading");
   uint8_t data [2];
   if (i2c_read(rel_module.cfg.address + 1, data, 2) == I2C_STATUS_OK)
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
         rel_update_relays_state(rel_module.current_relays, rel_module.current_status);
      }
   }
}
void rel_update_relays_state(uint16_t relays, RELAY_STATUS* status)
{
   for (uint8_t i = 0; i < RELAYS_BOARD_COUNT; i++)
   {
      RELAY_STATUS rel = {};
      rel.id = rel_module.cfg.items[i].id;
      rel.state = (relays & rel_id_to_mask(rel_module.cfg.items[i].id)) > 0? RELAY_STATE_ON : RELAY_STATE_OFF;

      if (status[i].id != rel.id || status[i].state != rel.state)
      {
         status[i] = rel;
         rel_notify_change(&rel);
      }
   }
}

void rel_notify_change(const RELAY_STATUS* status)
{
   for (uint8_t i = 0; i < RELAYS_CALLBACK_MAX_SIZE; i++)
   {
      if (REL_CALLBACKS[i])
      {
         REL_CALLBACKS[i](status);
      }
   }
}

RET_CODE rel_add_listener(REL_CALLBACK clb)
{
   RET_CODE result = RETURN_NOK;
   for (uint8_t i = 0; i < RELAYS_CALLBACK_MAX_SIZE; i++)
   {
      if (REL_CALLBACKS[i] == NULL)
      {
         REL_CALLBACKS[i] = clb;
         result = RETURN_OK;
         break;
      }
   }
   return result;
}
RET_CODE rel_remove_listener(REL_CALLBACK clb)
{
   RET_CODE result = RETURN_NOK;
   for (uint8_t i = 0; i < RELAYS_CALLBACK_MAX_SIZE; i++)
   {
      if (REL_CALLBACKS[i] == clb)
      {
         REL_CALLBACKS[i] = NULL;
         result = RETURN_OK;
         break;
      }
   }
   return result;
}
