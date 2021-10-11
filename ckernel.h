#ifndef _MYDEF
#define _MYDEF

#include "include/boot_meta.h"
#include "include/gdt.h"
#include "include/idt.h"
#include "include/paging.h"
#include "include/vbe.h"
#define vbe_info ((vbe_mode_info_structure*)(0xC500))
#define boot_meta ((boot_meta_info_struct*)(0x0600))
#define gdt_info ((GDT*)0x270000)
#define idt_info ((IDT*)0x26F800)

#define kernel_page_directory ((PageDirectory*)0x26E000)
#define kernel_page_table_first ((PageTable*)0x26D000)
#endif