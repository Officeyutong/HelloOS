#ifndef _KPRINTF_INCLUDE
#define _KPRINTF_INCLUDE

#include <stdarg.h>

__attribute__((format(printf, 2, 3))) int sprintf(char* buf,
                                                  const char* format,
                                                  ...);
int svprintf(char* buf, const char* format, va_list list);
#endif