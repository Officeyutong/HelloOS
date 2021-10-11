#ifndef _GDT_INCLUDE
#define _GDT_INCLUDE
#include <cinttypes>
#define GDT_COUNT ((uint32_t)8192)
struct GDT {
    char entry[8];
} __attribute__((packed, aligned(1)));

void write_segment_entry(void* ptr,
                         uint32_t base,
                         uint32_t limit,
                         uint8_t flags,
                         uint8_t access_byte);

#endif