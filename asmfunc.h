#ifndef _ASM_FUNC
#include "ckernel.h"
extern "C" {
void io_halt(void);
void write_global_uint32(uint32*, uint32);
void load_gdt(uint16 length, void* addr);
void load_idt(uint16 length, void* addr);

}
#define _ASM_FUNC
#endif