#ifndef _RETURN_CODES_H_
#define _RETURN_CODES_H_

/* ============================= */
/**
 * @file bt_engine.h
 *
 * @brief Return codes used in many functions
 *
 * @author Jacek Skowronek
 * @date 13/12/2020
 */
/* ============================= */


typedef enum ReturnCode
{
	RETURN_NOK = 0x00,   /**< Function not executed correctly */
	RETURN_OK = 0x01,    /**< Function executed correctly */
	RETURN_ERROR = 0xFF, /**< Error occurred */
} RET_CODE;


#endif
