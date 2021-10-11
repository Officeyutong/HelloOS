#ifndef _IDT_INCLUDE
#define _IDT_INCLUDE
#include <cinttypes>

#define IDT_COUNT ((uint32_t)256)

struct IDT {
    char entry[8][IDT_COUNT];
} __attribute__((packed, aligned(1)));

void write_interrupt_entry(void* ptr,
                           uint32_t offset,
                           uint16_t selector,
                           uint8_t present,
                           uint8_t DPL,
                           uint8_t S,
                           uint8_t gate_type);

#endif