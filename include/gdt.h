#ifndef _GDT_INCLUDE
#define _GDT_INCLUDE
#include "inttypes.h"
#define GDT_COUNT ((uint32)8192)
struct GDT {
    char entry[8][GDT_COUNT];
} __attribute__((packed, aligned(1)));

void write_segment_entry(void* ptr,
                         uint32 base,
                         uint32 limit,
                         uint8 flags,
                         uint8 access_byte);

#endif