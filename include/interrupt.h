#ifndef _INTERRUPT_H
#define _INTERRUPT_H

extern "C" void asm_interrupt_0x21();
extern "C" void c_interrupt_0x21(void* esp);
extern "C" void asm_interrupt_0x27();
extern "C" void c_interrupt_0x27(void* esp);
extern "C" void asm_interrupt_0x2c();
extern "C" void c_interrupt_0x2c(void* esp);

#endif