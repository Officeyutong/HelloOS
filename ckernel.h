#ifndef _MYDEF
#define _MYDEF

#include "include/gdt.h"
#include "include/vbe.h"
#include "include/boot_meta.h"
#include "include/idt.h"

#define vbe_info ((vbe_mode_info_structure*)(0xC500))
#define boot_meta ((boot_meta_info_struct*)(0x0600))
#define gdt_info ((GDT*)0x270000)
#define idt_info ((IDT*)0x26F800)

#endif