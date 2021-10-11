#include "ckernel.h"
#include "asmfunc.h"
#include "include/kprintf.h"
#include "include/string.h"
/*
显存通常是0xfd00 0000
*/
void write_pixel_at(int x, int y, uint32 pix) {
    uint32* offset = (uint32*)(y * vbe_info->pitch + x * (vbe_info->bpp / 8) +
                               vbe_info->framebuffer);
    *offset = pix;
}
static uint32 seed = 0;

uint32 rand() {
    seed ^= seed << 16;
    seed ^= seed >> 5;
    seed ^= seed << 1;
    return seed;
}
uint64 makeTime() {
    const char* timestamp = __TIMESTAMP__;
    uint64 r = 0;
    for (auto c = timestamp; *c; c++)
        r = r * 10 + *c - '0';
    return r;
}

void init_gdt_idt() {
    for (int i = 0; i < GDT_COUNT; i++)
        write_segment_entry(gdt_info + i, 0, 0, 0, 0);
    // write_segment_entry(gdt_info , 0, 0, 0, 0);                  // 0项
    write_segment_entry(gdt_info + 1, 0, 0xfffff, 0x9a, 0x0c);  // 全局可执行
    write_segment_entry(gdt_info + 2, 0, 0xfffff, 0x9a, 0x0c);  // 全局不可执行
    load_gdt(3 * 8 - 1, gdt_info);

    for (int i = 0; i < IDT_COUNT; i++)
        write_interrupt_entry(idt_info + i, 0, 0, 0, 0, 0, 0);
    load_idt(8 * IDT_COUNT - 1, idt_info);
}

void paint_screen() {
    seed = makeTime();
    const int SECTION_COUNT = 32;
    int sectionHeight = vbe_info->height / SECTION_COUNT;
    int seccolors[] = {0xffff00, 0xff00ff, 0x00ffff};
    for (int a = 0; a < SECTION_COUNT; a++) {
        for (int k = 0; k < SECTION_COUNT; k++) {
            uint32 color = seccolors[rand() % 3];
            for (int i = a * vbe_info->width / SECTION_COUNT;
                 i < (a + 1) * vbe_info->width / SECTION_COUNT; i++) {
                for (int j = k * sectionHeight; j < (k + 1) * sectionHeight;
                     j++) {
                    // 从右到左压栈
                    write_pixel_at(i, j, color);
                }
            }
        }
    }
}

extern "C" void kernel_main() {
    // write_pixel_at(0, 0, 0xff);
    init_gdt_idt();
    paint_screen();
    char s1[100] = "abcdefg";
    char s2[100] = "|bcdefgh";
    char buf[100];
    // int arr[2] = {0, 0};
    // arr[0] = strlen(s1);
    // arr[1] = strlen(s2);
    // strcat(s1, s2);
    sprintf(buf, "%d,%s", 123, "qwq");
    while (true) {
        __asm__("hlt");
    }
    // // 设置段表
}

void write_segment_entry(void* ptr,
                         uint32 base,
                         uint32 limit,
                         uint8 flags,
                         uint8 access_byte) {
    uint64 u = 0;
    u = u | (limit & 0xffff);
    u = u | ((((uint64)limit) >> 16) & 0xf) << 48;

    u = u | (((uint64)base & 0xffffff) << 16);
    u = u | ((((uint64)base >> 24) & 0xff) << 56);

    u = u | (((uint64)access_byte) << 40);

    u = u | (((uint64)flags & 0x0f) << 52);

    *((uint64*)ptr) = u;
}

void write_interrupt_entry(void* ptr,
                           uint32 offset,
                           uint16 selector,
                           uint8 present,
                           uint8 DPL,
                           uint8 S,
                           uint8 gate_type) {
    uint64 u = 0;
    // 写offset
    u |= (offset & 0xffff);
    u |= (((uint64)offset >> 16) << 48);
    // 写selector
    u |= (selector << 16);
    // 拼接type并写进去
    uint8 type = 0;
    type |= ((present & 1) << 7);
    type |= ((DPL & 3) << 5);
    type |= ((S & 1) << 4);
    type |= (gate_type & 0x0f);
    u |= ((uint64)type << 40);
    *((uint64*)ptr) = u;
}