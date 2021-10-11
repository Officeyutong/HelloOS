#ifndef _ASM_FUNC
#include <cinttypes>
extern "C" {
void io_halt(void);
void write_global_uint32(uint32_t*, uint32_t);
void load_gdt(uint16_t length, void* addr);
void load_idt(uint16_t length, void* addr);
}
#define _ASM_FUNC
#endif