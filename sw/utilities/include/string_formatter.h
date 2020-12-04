#ifndef _STRING_FORMATTER_H_
#define _STRING_FORMATTER_H_

#include <stdarg.h>

void ts_itoa(char **buf, unsigned int d, int base);
int ts_formatstring(char *buf, const char *fmt, va_list va);
int ts_formatlength(const char *fmt, va_list va);
int string_format (char* buffer, const char *fmt, ...);


#endif  // _STRING_FORMATTER_H_
