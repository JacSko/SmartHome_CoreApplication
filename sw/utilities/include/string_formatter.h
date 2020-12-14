#ifndef _STRING_FORMATTER_H_
#define _STRING_FORMATTER_H_
/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdarg.h>

/**
 * @brief Convert integer value to ASCII.
 * @param[out] buf - Place to store integer string
 * @param[in] d - Integer to convert
 * @param[in] base - conversion base
 * @return None.
 */
void ts_itoa(char **buf, unsigned int d, int base);
/**
 * @brief Format string according to provided format.
 * @param[out] buf - Place to store prepared string
 * @param[in] fmt - Desired format
 * @param[in] va - list of arguments
 * @return How many bytes written.
 */
int ts_formatstring(char *buf, const char *fmt, va_list va);
/**
 * @brief Check length of formatted string.
 * @param[in] fmt - Desired format
 * @param[in] va - list of arguments
 * @return Length of formatted string.
 */
int ts_formatlength(const char *fmt, va_list va);
/**
 * @brief Format string with variable arguments.
 * @param[out] buf - Place to store prepared string
 * @param[in] fmt - Desired format
 * @param[in] va - arguments
 * @return Length of formatted string.
 */
int string_format (char* buffer, const char *fmt, ...);


#endif
