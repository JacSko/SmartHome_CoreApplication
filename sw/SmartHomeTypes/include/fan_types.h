#ifndef _FAN_TYPES_H_
#define _FAN_TYPES_H_

/* ============================= */
/**
 * @file fan_types.h
 *
 * @brief Common enum types related to Fan module.
 *
 * @author Jacek Skowronek
 * @date 23/01/2021
 */
/* ============================= */

typedef enum FAN_STATE
{
   FAN_STATE_OFF,       /**< Fan is stopped */
   FAN_STATE_ON,        /**< Fan is running*/
   FAN_STATE_SUSPEND,   /**< Fan blocked */
   FAN_STATE_UNKNOWN,   /**< Unknown state */
} FAN_STATE;

#endif
