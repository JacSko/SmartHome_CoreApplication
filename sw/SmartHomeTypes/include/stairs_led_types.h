#ifndef _STAIRS_LED_TYPES_H_
#define _STAIRS_LED_TYPES_H_

/* ============================= */
/**
 * @file stairs_led_types.h
 *
 * @brief Common enum types related to StairsLedModule module.
 *
 * @author Jacek Skowronek
 * @date 23/01/2021
 */
/* ============================= */

typedef enum SLM_STATE
{
   SLM_STATE_OFF,                /**< Module is in idle state, ready to work */
   SLM_STATE_ONGOING_ON,         /**< Program is running, leds goes forward */
   SLM_STATE_ON,                 /**< All LEDs are ON */
   SLM_STATE_OFF_EFFECT,         /**< OFF effect is active */
   SLM_STATE_OFF_EFFECT_READY,   /**< OFF effect is done */
   SLM_STATE_ONGOING_OFF,        /**< Program is running, leds goes backwards */
} SLM_STATE;
typedef enum SLM_PROGRAM_ID
{
   SLM_PROGRAM1,
   SLM_PROGRAM2,
   SLM_PROGRAM3,
} SLM_PROGRAM_ID;
typedef enum SLM_OFF_EFFECT_MODE
{
   SLM_OFF_EFFECT_DISABLED,   /**< Leds are disabled directly after timeout */
   SLM_OFF_EFFECT_ENABLED,    /**< Before switching off the leds, another effect is run */
} SLM_OFF_EFFECT_MODE;


#endif
