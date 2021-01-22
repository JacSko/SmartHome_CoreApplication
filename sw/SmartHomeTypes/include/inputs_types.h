#ifndef _INPUTS_TYPES_H_
#define _INPUTS_TYPES_H_

/* ============================= */
/**
 * @file inputs_types.h
 *
 * @brief Common enum types related to Inputs module.
 *
 * @author Jacek Skowronek
 * @date 20/12/2020
 */
/* ============================= */


typedef enum INPUT_STATE
{
   INPUT_STATE_INACTIVE, /**< Input is inactive */
   INPUT_STATE_ACTIVE,   /**< Input is active */
} INPUT_STATE;
typedef enum INPUT_ID
{
   INPUT_WARDROBE_AC,  /**< Wardrobe light on the ceiling */
   INPUT_WARDROBE_LED, /**< Wardrobe LED light */
   INPUT_BEDROOM_AC,   /**< Bedroom light on the ceiling */
   INPUT_BATHROOM_AC,  /**< Bathroom light on the ceiling */
   INPUT_BATHROOM_LED, /**< Bathroom LED light */
   INPUT_KITCHEN_AC,   /**< Kitchen light on the ceiling */
   INPUT_KITCHEN_WALL, /**< Kitchen light on the wall */
   INPUT_STAIRS_AC,    /**< Stairs light on the ceiling */
   INPUT_STAIRS_SENSOR,/**< The sensor on staircase */
   INPUT_SOCKETS,      /**< AC sockets power */
   INPUT_ENUM_COUNT,   /**< Enumeration count */
} INPUT_ID;




#endif
