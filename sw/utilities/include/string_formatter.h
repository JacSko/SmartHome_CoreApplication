#ifndef _STRING_FORMATTER_H_
#define _STRING_FORMATTER_H_
/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdarg.h>

/* ============================= */
/**
 * @file string_formatter.h
 *
 * @brief Set of functions which allows to format strings
 *
 * @author Jacek Skowronek
 * @date 13/12/2020
 */
/* ============================= */

/**
 * @brief Convert integer value to ASCII.
 * @param[out] buf - Place to store integer string
 * @param[in] d - Integer to convert
 * @param[in] base - conversion base
 * @return None.
 */
void sf_itoa(char **buf, unsigned int d, int base);
/**
 * @brief Format string according to provided format.
 * @param[out] buf - Place to store prepared string
 * @param[in] fmt - Desired format
 * @param[in] va - list of arguments
 * @return How many bytes written.
 */
int sf_format_string(char *buf, const char *fmt, va_list va);
/**
 * @brief Check length of formatted string.
 * @param[in] fmt - Desired format
 * @param[in] va - list of arguments
 * @return Length of formatted string.
 */
//int sf_format_length(const char *fmt, va_list va);
/**
 * @brief Format string with variable arguments.
 * @param[out] buf - Place to store prepared string
 * @param[in] fmt - Desired format
 * @param[in] va - arguments
 * @return Length of formatted string.
 */
int string_format (char* buffer, const char *fmt, ...);


#endif
