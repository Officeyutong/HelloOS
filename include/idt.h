#ifndef _IDT_INCLUDE
#define _IDT_INCLUDE
#include "inttypes.h"

#define IDT_COUNT ((uint32)256)

struct IDT {
    char entry[8][IDT_COUNT];
} __attribute__((packed, aligned(1)));

void write_interrupt_entry(void* ptr,
                           uint32 offset,
                           uint16 selector,
                           uint8 present,
                           uint8 DPL,
                           uint8 S,
                           uint8 gate_type);

#endif