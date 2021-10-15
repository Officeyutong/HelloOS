#ifndef _MYDEF
#define _MYDEF

#include "include/ascii_font.h"
#include "include/boot_meta.h"
#include "include/gdt.h"
#include "include/idt.h"
#include "include/paging.h"
#include "include/vbe.h"
#define vbe_info ((vbe_mode_info_structure*)(0xE600))
#define boot_meta ((boot_meta_info_struct*)(0x0600))
#define boot_sector ((FAT32BootSector*)0x7C00)
#define gdt_info ((GDT*)0x270000)
#define idt_info ((IDT*)0x26F800)

#define kernel_page_directory ((PageDirectory*)0x26E000)
#define kernel_page_table_first ((PageTable*)0x26C000)
#define ascii_font_table ((ASCIIFontTable*)0x26D000)

extern PageTable* next_kernel_page_table;

#endif