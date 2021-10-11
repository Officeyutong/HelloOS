#ifndef _STRING_INCLUDE
#define _STRING_INCLUDE
#include "inttypes.h"
void* memset(void* src, int val, size_t cnt);
void* memcpy(void* dest, const void* src, size_t cnt);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
size_t strlen(const char* s);
#endif
