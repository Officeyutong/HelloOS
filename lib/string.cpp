#include "../include/string.h"
#include <inttypes.h>
void* memset(void* src, int val, size_t cnt) {
    char* p = (char*)src;
    for (int i = 0; i < cnt; i++) {
        *p = val;
        p++;
    }
    return src;
}
void* memcpy(void* dest, const void* src, size_t cnt) {
    const char* s = (const char*)src;
    char* d = (char*)dest;
    for (int i = 0; i < cnt; i++) {
        *d = *s;
        s++, d++;
    }
    // asm("movl %2, %%ecx;"
    //     "movl %0, %%esi;"
    //     "movl %1, %%edi;"
    //     "cld;"
    //     "rep movsb;" ::""(dest),
    //     ""(src), ""(cnt)
    //     : "esi", "edi", "ecx");
    // return dest;
}
char* strcpy(char* dest, const char* src) {
    char* p1 = dest;
    const char* p2 = src;
    while (*p2) {
        *p1 = *p2;
        p1++, p2++;
    }
    *p1 = '\0';
    return dest;
}
char* strcat(char* dest, const char* src) {
    char* p = dest;
    while (*p)
        p++;
    const char* q = src;
    while (*q) {
        *p = *q;
        q++, p++;
    }
    p++;
    *p = 0;
    return dest;
}
size_t strlen(const char* s) {
    size_t ret = 0;
    while (*s) {
        ret++;
        s++;
    }
    return ret;
}
