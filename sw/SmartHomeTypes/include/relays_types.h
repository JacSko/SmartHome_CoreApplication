#ifndef _RELAYS_TYPES_H_
#define _RELAYS_TYPES_H_

/* ============================= */
/**
 * @file relays_types.h
 *
 * @brief Common enum types related to Relays module.
 *
 * @author Jacek Skowronek
 * @date 23/01/2021
 */
/* ============================= */


typedef enum RELAY_STATE
{
   RELAY_STATE_OFF,           /**< Relay is turned OFF */
   RELAY_STATE_ON,            /**< Relay is turned ON */
   RELAY_STATE_ENUM_MAX,      /**< Count of enum items */
} RELAY_STATE;
typedef enum RELAY_ID
{
   RELAY_WARDROBE_LED = 1,
   RELAY_WARDROBE_AC,
   RELAY_BATHROOM_LED,
   RELAY_BATHROOM_AC,
   RELAY_STAIRCASE_LED,
   RELAY_STAIRCASE_AC,
   RELAY_SOCKETS,
   RELAY_KITCHEN_WALL,
   RELAY_KITCHEN_AC,
   RELAY_BEDROOM_AC,
   RELAY_BATHROOM_FAN,
   RELAY_ID_ENUM_MAX,
} RELAY_ID;



#endif
