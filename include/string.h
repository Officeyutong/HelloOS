#ifndef _STRING_INCLUDE
#define _STRING_INCLUDE
#include <cinttypes>
#include <cstddef>
void* memset(void* src, int val, size_t cnt);
void* memcpy(void* dest, const void* src, size_t cnt);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
size_t strlen(const char* s);
int strcmp(const char* s1, const char* s2);
#endif
