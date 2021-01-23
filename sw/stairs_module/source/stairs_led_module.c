/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdlib.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "stairs_led_module.h"
#include "task_scheduler.h"
#include "i2c_driver.h"
#include "Logger.h"
#include "inputs_board.h"
/* =============================
 *          Defines
 * =============================*/
#define SLM_MAX_LISTENERS 5
/* =============================
 *       Internal types
 * =============================*/
typedef enum SLM_REQ_TYPE
{
   SLM_REQ_NORMAL,
   SLM_REQ_ALW_ON,
} SLM_REQ_TYPE;
typedef struct SLM_MODULE
{
   SLM_PROGRAM* program;
   SLM_STEP* step;
   uint8_t step_id;
   SLM_STATE state;
   SLM_REQ_TYPE type;
   SLM_CONFIG cfg;
} SLM_MODULE;
/* =============================
 *   Internal module functions
 * =============================*/
void slm_prepare_default_programs();
void slm_on_timeout();
void slm_send_step(const SLM_STEP* step);
void slm_run_program(SLM_REQ_TYPE type);
RET_CODE slm_is_program_valid(const SLM_PROGRAM* program);
void slm_on_sensor_state_change(INPUT_STATUS status);
void slm_set_state(SLM_STATE state);
void slm_notify_change();
/* =============================
 *      Module variables
 * =============================*/
SLM_PROGRAM LED_PROGRAMS[SLM_MAX_PROGRAMS];
SLM_MODULE led_module;
void (*SLM_CALLBACKS[SLM_MAX_LISTENERS])(SLM_STATE);
RET_CODE slm_initialize(const SLM_CONFIG* config)
{
   RET_CODE result = RETURN_NOK;
   slm_prepare_default_programs();

   logger_send(LOG_SLM, __func__,"");
   if (config)
   {
      led_module.cfg = *config;
      led_module.program = &LED_PROGRAMS[led_module.cfg.program_id];
      led_module.step_id = 0;
      led_module.step = &(led_module.program->program_steps[led_module.step_id]);
      led_module.state = SLM_STATE_OFF;
      led_module.type = SLM_REQ_NORMAL;

      slm_send_step(led_module.step); /* turn off all LEDs*/

      result = sch_subscribe_and_set(&slm_on_timeout, TASKPRIO_LOW, 1000, TASKSTATE_STOPPED, TASKTYPE_TRIGGER);
      if (result == RETURN_OK)
      {
         result = inp_add_input_listener(&slm_on_sensor_state_change);
         logger_send(LOG_SLM, __func__,"init OK");
      }
      for (uint8_t i = 0; i < SLM_MAX_LISTENERS; i++)
      {
         SLM_CALLBACKS[i] = NULL;
      }
   }

   return result;
}

void slm_on_timeout()
{
   logger_send(LOG_SLM, __func__,"state: %d", led_module.state);
   switch(led_module.state)
   {
   case SLM_STATE_OFF:
      sch_set_task_state(&slm_on_timeout, TASKSTATE_STOPPED);
      break;
   case SLM_STATE_ON:
      if (inp_get(INPUT_STAIRS_SENSOR) == INPUT_STATE_INACTIVE)
      {
         if (led_module.cfg.off_effect_mode == SLM_OFF_EFFECT_ENABLED)
         {
            slm_set_state(SLM_STATE_OFF_EFFECT);
            led_module.step_id = 0;
            led_module.step = &(led_module.program->off_effect_steps[led_module.step_id]);
         }
         else
         {
            slm_set_state(SLM_STATE_ONGOING_OFF);
            led_module.step_id = led_module.program->program_steps_count - 2;
            led_module.step = &(led_module.program->program_steps[led_module.step_id]);
         }
         slm_send_step(led_module.step);
      }
      else
      {
         sch_trigger_task(&slm_on_timeout);
      }
      break;
   case SLM_STATE_OFF_EFFECT:
      led_module.step_id++;
      if (led_module.step_id == led_module.program->off_effect_steps_count - 1)
      {
         slm_set_state(SLM_STATE_OFF_EFFECT_READY);
      }
      led_module.step = &(led_module.program->off_effect_steps[led_module.step_id]);
      slm_send_step(led_module.step);
      break;
   case SLM_STATE_OFF_EFFECT_READY:
      led_module.step_id = led_module.program->program_steps_count - 2;
      slm_set_state(SLM_STATE_ONGOING_OFF);
      led_module.step = &(led_module.program->program_steps[led_module.step_id]);
      slm_send_step(led_module.step);
      break;
   case SLM_STATE_ONGOING_OFF:
      led_module.step_id--;
      if (led_module.step_id == 0)
      {
         slm_set_state(SLM_STATE_OFF);
      }
      led_module.step = &(led_module.program->program_steps[led_module.step_id]);
      slm_send_step(led_module.step);
      break;
   case SLM_STATE_ONGOING_ON:
      led_module.step_id++;
      if (led_module.step_id == led_module.program->program_steps_count - 1)
      {
         slm_set_state(SLM_STATE_ON);
      }
      led_module.step = &(led_module.program->program_steps[led_module.step_id]);
      slm_send_step(led_module.step);

      if (led_module.type == SLM_REQ_ALW_ON && led_module.state == SLM_STATE_ON)
      {
         sch_set_task_state(&slm_on_timeout, TASKSTATE_STOPPED);
      }
      break;

   }
}
void slm_on_sensor_state_change(INPUT_STATUS status)
{
   if (status.id == INPUT_STAIRS_SENSOR && status.state == INPUT_STATE_ACTIVE)
   {
      logger_send(LOG_SLM, __func__,"state %d", led_module.state);
      switch(led_module.state)
      {
      case SLM_STATE_OFF:
         slm_run_program(SLM_REQ_NORMAL);
         break;
      case SLM_STATE_ONGOING_OFF:
         slm_set_state(SLM_STATE_ONGOING_ON);
         slm_send_step(led_module.step);
         break;
      case SLM_STATE_OFF_EFFECT:
      case SLM_STATE_OFF_EFFECT_READY:
         slm_set_state(SLM_STATE_ON);
         led_module.step_id = led_module.program->program_steps_count - 1;
         led_module.step = &(led_module.program->program_steps[led_module.step_id]);
         slm_send_step(led_module.step);
         break;
      default:
         break;
      }
   }
}
void slm_deinitialize()
{
   for (uint8_t i = 0; i < SLM_MAX_LISTENERS; i++)
   {
      SLM_CALLBACKS[i] = NULL;
   }
}
void slm_prepare_default_programs()
{
   LED_PROGRAMS[SLM_PROGRAM1].program_steps_count = 9;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[0].leds_state = 0b00000000;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[0].period = 0;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[1].leds_state = 0b00000001;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[1].period = 20;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[2].leds_state = 0b00000011;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[2].period = 20;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[3].leds_state = 0b00000111;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[3].period = 20;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[4].leds_state = 0b00001111;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[4].period = 20;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[5].leds_state = 0b00011111;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[5].period = 20;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[6].leds_state = 0b00111111;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[6].period = 20;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[7].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[7].period = 20;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[8].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM1].program_steps[8].period = 20000;

   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps_count = 10;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[0].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[0].period = 200;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[1].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[1].period = 200;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[2].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[2].period = 200;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[3].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[3].period = 200;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[4].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[4].period = 200;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[5].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[5].period = 200;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[6].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[6].period = 200;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[7].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[7].period = 200;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[8].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[8].period = 200;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[9].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM1].off_effect_steps[9].period = 200;


   LED_PROGRAMS[SLM_PROGRAM2].program_steps_count = 5;
   LED_PROGRAMS[SLM_PROGRAM2].program_steps[0].leds_state = 0b00000000;
   LED_PROGRAMS[SLM_PROGRAM2].program_steps[0].period = 0;
   LED_PROGRAMS[SLM_PROGRAM2].program_steps[1].leds_state = 0b10000001;
   LED_PROGRAMS[SLM_PROGRAM2].program_steps[1].period = 20;
   LED_PROGRAMS[SLM_PROGRAM2].program_steps[2].leds_state = 0b11000011;
   LED_PROGRAMS[SLM_PROGRAM2].program_steps[2].period = 20;
   LED_PROGRAMS[SLM_PROGRAM2].program_steps[3].leds_state = 0b11100111;
   LED_PROGRAMS[SLM_PROGRAM2].program_steps[3].period = 20;
   LED_PROGRAMS[SLM_PROGRAM2].program_steps[4].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM2].program_steps[4].period = 20000;

   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps_count = 10;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[0].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[0].period = 200;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[1].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[1].period = 200;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[2].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[2].period = 200;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[3].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[3].period = 200;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[4].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[4].period = 200;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[5].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[5].period = 200;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[6].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[6].period = 200;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[7].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[7].period = 200;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[8].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[8].period = 200;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[9].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM2].off_effect_steps[9].period = 200;


   LED_PROGRAMS[SLM_PROGRAM3].program_steps_count = 5;
   LED_PROGRAMS[SLM_PROGRAM3].program_steps[0].leds_state = 0b00000000;
   LED_PROGRAMS[SLM_PROGRAM3].program_steps[0].period = 0;
   LED_PROGRAMS[SLM_PROGRAM3].program_steps[1].leds_state = 0b00000011;
   LED_PROGRAMS[SLM_PROGRAM3].program_steps[1].period = 20;
   LED_PROGRAMS[SLM_PROGRAM3].program_steps[2].leds_state = 0b00001111;
   LED_PROGRAMS[SLM_PROGRAM3].program_steps[2].period = 20;
   LED_PROGRAMS[SLM_PROGRAM3].program_steps[3].leds_state = 0b00111111;
   LED_PROGRAMS[SLM_PROGRAM3].program_steps[3].period = 20;
   LED_PROGRAMS[SLM_PROGRAM3].program_steps[4].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM3].program_steps[4].period = 20000;

   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps_count = 10;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[0].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[0].period = 200;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[1].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[1].period = 200;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[2].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[2].period = 200;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[3].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[3].period = 200;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[4].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[4].period = 200;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[5].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[5].period = 200;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[6].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[6].period = 200;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[7].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[7].period = 200;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[8].leds_state = 0b01111111;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[8].period = 200;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[9].leds_state = 0b11111111;
   LED_PROGRAMS[SLM_PROGRAM3].off_effect_steps[9].period = 200;

}
void slm_send_step(const SLM_STEP* step)
{
   i2c_write(led_module.cfg.address, &(step->leds_state), 2);
   sch_set_task_period(&slm_on_timeout, step->period);
   sch_trigger_task(&slm_on_timeout);
}
RET_CODE slm_get_config(SLM_CONFIG* buffer)
{
   RET_CODE result = RETURN_NOK;
   if (buffer)
   {
      *buffer = led_module.cfg;
      result = RETURN_OK;
   }
   return result;
}
RET_CODE slm_start_program_alw_on()
{
   logger_send(LOG_SLM, __func__,"");
   RET_CODE result = RETURN_NOK;
   if (slm_get_state() == SLM_STATE_OFF)
   {
      slm_run_program(SLM_REQ_ALW_ON);
      result = RETURN_OK;
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "invalid state");
   return result;
}
RET_CODE slm_start_program()
{
   logger_send(LOG_SLM, __func__,"");
   RET_CODE result = RETURN_NOK;
   if (slm_get_state() == SLM_STATE_OFF)
   {
      slm_run_program(SLM_REQ_NORMAL);
      result = RETURN_OK;
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "invalid state");
   return result;
}
void slm_run_program(SLM_REQ_TYPE type)
{
   logger_send(LOG_SLM, __func__,"");
   led_module.type = type;
   slm_set_state(SLM_STATE_ONGOING_ON);
   led_module.program = &LED_PROGRAMS[led_module.cfg.program_id];
   led_module.step_id = 1;
   led_module.step = &(led_module.program->program_steps[led_module.step_id]);
   slm_send_step(led_module.step);
}
void slm_set_state(SLM_STATE state)
{
   led_module.state = state;
   slm_notify_change();
}
RET_CODE slm_stop_program()
{
   logger_send(LOG_SLM, __func__,"");
   RET_CODE result = RETURN_NOK;
   if (led_module.state == SLM_STATE_ON)
   {
      if (led_module.cfg.off_effect_mode == SLM_OFF_EFFECT_ENABLED)
      {
         slm_set_state(SLM_STATE_OFF_EFFECT);
         led_module.step_id = 0;
         led_module.step = &(led_module.program->off_effect_steps[led_module.step_id]);
         slm_send_step(led_module.step);
         logger_send(LOG_SLM, __func__,"started off effect");
      }
      else
      {
         slm_set_state(SLM_STATE_ONGOING_OFF);
         led_module.step_id = led_module.program->program_steps_count - 2;
         led_module.step = &(led_module.program->program_steps[led_module.step_id]);
         slm_send_step(led_module.step);
         logger_send(LOG_SLM, __func__,"started disabling");
      }
      result = RETURN_OK;
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__,"invalid state");
   return result;
}
RET_CODE slm_add_listener(SLM_LISTENER listener)
{
   RET_CODE result = RETURN_NOK;
   for (uint8_t i = 0; i < SLM_MAX_LISTENERS; i++)
   {
      if (SLM_CALLBACKS[i] == NULL)
      {
         SLM_CALLBACKS[i] = listener;
         result = RETURN_OK;
         break;
      }
   }
   return result;
}
RET_CODE slm_remove_listener(SLM_LISTENER listener)
{
   RET_CODE result = RETURN_NOK;
   for (uint8_t i = 0; i < SLM_MAX_LISTENERS; i++)
   {
      if (SLM_CALLBACKS[i] == listener)
      {
         SLM_CALLBACKS[i] = NULL;
         result = RETURN_OK;
         break;
      }
   }
   return result;
}
void slm_notify_change()
{
   for (uint8_t i = 0; i < SLM_MAX_LISTENERS; i++)
   {
      if (SLM_CALLBACKS[i])
      {
         SLM_CALLBACKS[i](led_module.state);
      }
   }
}
SLM_STATE slm_get_state()
{
   return led_module.state;
}
SLM_PROGRAM_ID slm_get_current_program_id()
{
   return led_module.cfg.program_id;
}
RET_CODE slm_set_current_program_id(SLM_PROGRAM_ID id)
{
   RET_CODE result = RETURN_NOK;
   if (id <= SLM_PROGRAM3 && led_module.state == SLM_STATE_OFF)
   {
      led_module.cfg.program_id = id;
      result = RETURN_OK;
      logger_send(LOG_SLM, __func__,"new program id %d", id);
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__,"invalid program id %d", id);
   return result;
}
RET_CODE slm_get_program_by_id(SLM_PROGRAM_ID id, SLM_PROGRAM* buffer)
{
   RET_CODE result = RETURN_NOK;
   if (buffer && id <= SLM_PROGRAM3)
   {
      *buffer = LED_PROGRAMS[id];
      result = RETURN_OK;
   }
   return result;
}

RET_CODE slm_is_program_valid(const SLM_PROGRAM* program)
{
   RET_CODE result = RETURN_OK;
   for (uint8_t i = 0; i < program->program_steps_count; i++)
   {
      if (program->program_steps[i].period < 10)
      {
         logger_send(LOG_SLM, __func__,"invalid period, program step %d", i);
         result = RETURN_NOK;
         break;
      }
   }

   for (uint8_t i = 0; i < program->off_effect_steps_count; i++)
   {
      if (program->off_effect_steps[i].period < 10)
      {
         logger_send(LOG_SLM, __func__,"invalid period, off program step %d", i);
         result = RETURN_NOK;
         break;
      }
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "invalid program");
   return result;
}

RET_CODE slm_replace_program(SLM_PROGRAM_ID id, const SLM_PROGRAM* program)
{
   RET_CODE result = RETURN_NOK;
   if (program && id <= SLM_PROGRAM3)
   {
      if ((id != slm_get_current_program_id()) ||
          (id == slm_get_current_program_id() && led_module.state == SLM_STATE_OFF))
      {
         if (slm_is_program_valid(program) == RETURN_OK)
         {
            logger_send(LOG_SLM, __func__,"program %d replaced", id);
            LED_PROGRAMS[id] = *program;
            result = RETURN_OK;
         }
      }
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "cannot replace program %d", id);
   return result;
}


