/* =============================
 *  Includes of project headers
 * =============================*/
#include "string_formatter.h"


void sf_itoa(char **buf, unsigned int d, int base)
{
	int div = 1;
	while (d/div >= base)
		div *= base;

	while (div != 0)
	{
		int num = d/div;
		d = d%div;
		div /= base;
		if (num > 9)
			*((*buf)++) = (num-10) + 'A';
		else
			*((*buf)++) = num + '0';
	}
}

int sf_atoi(const char *string)
{
    int res = 0;
    int sign = 1;
    if (*string == '-')
    {
        sign = -1;
        string++;
    }
    if (*string == '+') string++;

    while (*string >= '0' && *string <= '9')
    {
        res = res * 10 + (*string - '0');
        string++;
    }
    return (sign < 0) ? (-res) : res;
}

int sf_count_number(int n)
{
    int count = 0;
    while (n != 0)
    {
        count++;
        n /= 10;
    }
    return count;
}

int sf_format_string(char *buf, const char *fmt, va_list va)
{
	char *start_buf = buf;
	unsigned int precision = 0;
	while(*fmt)
	{
		/* Character needs formating? */
		if (*fmt == '%')
		{
		    ++fmt;
		    if (*(fmt) == '.')
		    {
		        ++fmt;
		        //means that precision requested
		        precision = sf_atoi(fmt);
		        fmt++;
		    }
		    switch (*(fmt))
			{
			  case 'c':
				*buf++ = va_arg(va, int);
				break;
			  case 'd':
			  case 'i':
				{
					signed int val = va_arg(va, signed int);
					while (precision > (unsigned int)sf_count_number(val))
					{
					    *buf++ = '0';
					    precision--;
					}
					if (val < 0)
					{
						val *= -1;
						*buf++ = '-';
					}
					sf_itoa(&buf, val, 10);
				}
				break;
			  case 's':
				{
					char * arg = va_arg(va, char *);
					while (*arg)
					{
						*buf++ = *arg++;
					}
				}
				break;
			  case 'u':
			  {
			        unsigned int val = va_arg(va, unsigned int);
			        while (precision > (unsigned int)sf_count_number(val))
					{
					    *buf++ = '0';
					    precision--;
					}
					sf_itoa(&buf, val, 10);
			  }
				break;
			  case 'x':
			  case 'X':
					sf_itoa(&buf, va_arg(va, int), 16);
				break;
			  case '%':
				  *buf++ = '%';
				  break;
			}
			fmt++;
		}
		/* Else just copy */
		else
		{
			*buf++ = *fmt++;
		}
	}
	*buf = 0;

	return (int)(buf - start_buf);
}

int sf_format_length(const char *fmt, va_list va)
{
	int length = 0;
	while (*fmt)
	{
		if (*fmt == '%')
		{
			++fmt;
			switch (*fmt)
			{
			  case 'c':
		  		  va_arg(va, int);
				  ++length;
				  break;
			  case 'd':
			  case 'i':
			  case 'u':
				  /* 32 bits integer is max 11 characters with minus sign */
				  length += 11;
				  va_arg(va, int);
				  break;
			  case 's':
			  	  {
			  		  char * str = va_arg(va, char *);
			  		  while (*str++)
			  			  ++length;
			  	  }
				  break;
			  case 'x':
			  case 'X':
				  /* 32 bits integer as hex is max 8 characters */
				  length += 8;
				  va_arg(va, unsigned int);
				  break;
			  default:
				  ++length;
				  break;
			}
		}
		else
		{
			++length;
		}
		++fmt;
	}
	return length;
}

int string_format (char* buffer, const char *fmt, ...)
{
	int result = 0;
	va_list va;
	va_start(va, fmt);
	result = sf_format_string(buffer, fmt, va);
	va_end(va);
	return result;
}
