#include "ckernel.h"
#include <inttypes.h>
#include "include/display.h"
#include "include/gdt.h"
#include "include/harddisk.h"
#include "include/idt.h"
#include "include/interrupt.h"
#include "include/keyboard_mouse.h"
#include "include/kprintf.h"
#include "include/kutil.h"
#include "include/string.h"
PageTable* next_kernel_page_table = kernel_page_table_first;
static uint32_t seed = 0;
static uint32_t rand() {
    seed ^= seed << 16;
    seed ^= seed >> 5;
    seed ^= seed << 1;
    return seed;
}
uint64_t makeTime() {
    const char* timestamp = __TIMESTAMP__;
    uint64_t r = 0;
    for (auto c = timestamp; *c; c++)
        r = r * 10 + *c - '0';
    return r;
}

static void init_gdt_idt() {
    for (int i = 0; i < GDT_COUNT; i++)
        write_segment_entry(gdt_info + i, 0, 0, 0, 0);
    // write_segment_entry(gdt_info , 0, 0, 0, 0);                  // 0项
    write_segment_entry(gdt_info + 1, 0, 0xfffff, 0x0c, 0x9a);  // 全局可执行
    write_segment_entry(gdt_info + 2, 0, 0xfffff, 0x0c, 0x92);  // 全局不可执行
    load_gdt(8 * GDT_COUNT - 1, gdt_info);

    for (int i = 0; i < IDT_COUNT; i++)
        write_interrupt_entry(idt_info + i, 0, 0, 0, 0, 0, 0);
    write_interrupt_entry(idt_info + 0x21, (uint32_t)&asm_interrupt_0x21,
                          1 << 3, 1, 0, 0, 0x0e);
    write_interrupt_entry(idt_info + 0x27, (uint32_t)&asm_interrupt_0x27,
                          1 << 3, 1, 0, 0, 0x0e);
    write_interrupt_entry(idt_info + 0x2c, (uint32_t)&asm_interrupt_0x2c,
                          1 << 3, 1, 0, 0, 0x0e);

    load_idt(8 * IDT_COUNT - 1, idt_info);
}

static void init_paging() {
    map_memory_page(0, 0xff);
    map_memory_page(0x260, 0x4ff);
    map_memory_page(0xfd000, 0xfd24f);
    asm("movl %0, %%eax;"
        "movl %%eax, %%cr3;"
        "movl %%cr0, %%eax;"
        "or $0x80000001, %%eax;"
        "movl %%eax, %%cr0;" ::""(kernel_page_directory)
        : "eax");
}
void paint_screen() {
    seed = makeTime();
    const int SECTION_COUNT = 32;
    int sectionHeight = vbe_info->height / SECTION_COUNT;
    int seccolors[] = {0xffff00, 0xff00ff, 0x00ffff};
    for (int a = 0; a < SECTION_COUNT; a++) {
        for (int k = 0; k < SECTION_COUNT; k++) {
            uint32_t color = seccolors[rand() % 3];
            for (int i = a * vbe_info->width / SECTION_COUNT;
                 i < (a + 1) * vbe_info->width / SECTION_COUNT; i++) {
                for (int j = k * sectionHeight; j < (k + 1) * sectionHeight;
                     j++) {
                    // write_pixel_at(i, j, 0x0);
                    write_pixel_at(i, j, color);
                }
            }
        }
    }
}
static void init_pic(uint8_t master_offset, uint8_t slave_offset) {
    // uint8_t a1 = io_in8(PIC1_DATA);
    // uint8_t a2 = io_in8(PIC2_DATA);
    io_out8(PIC1_DATA, 0xff);
    io_out8(PIC2_DATA, 0xff);

    io_out8(PIC1_COMMAND, 0x11);
    io_out8(PIC1_DATA, master_offset);
    io_out8(PIC1_DATA, 1 << 2);
    io_out8(PIC1_DATA, 1);

    io_out8(PIC2_COMMAND, 0x11);
    io_out8(PIC2_DATA, slave_offset);
    io_out8(PIC2_DATA, 2);
    io_out8(PIC2_DATA, 1);

    io_out8(PIC1_DATA, 0xfb);
    io_out8(PIC2_DATA, 0xff);
}

static void init_keyboard() {
    using namespace keyboard_mouse;
    keyboard_buffer.init();
    wait_for_keyboard_send_ready();
    io_out8(KEYBOARD_COMMAND, 0x60);
    wait_for_keyboard_send_ready();
    io_out8(KEYBOARD_DATA, 0x47);
}
static void init_mouse() {
    using namespace keyboard_mouse;
    mouse_buffer.init();
    wait_for_keyboard_send_ready();
    io_out8(KEYBOARD_COMMAND, 0xd4);
    wait_for_keyboard_send_ready();
    io_out8(KEYBOARD_DATA, 0xF4);
}
extern "C" __attribute__((section("section_kernel_main"))) void kernel_main() {
    init_gdt_idt();
    init_paging();
    init_pic(0x20, 0x28);
    io_sti();
    paint_screen();
    using namespace fat32;
    init_keyboard();
    FAT32Reader reader(boot_sector, boot_meta);
    DirectoryEntry file;
    auto start_cluster =
        reader.find_file(reader.info->root_cluster, "ascii_font.bin", file);
    reader.read_file(start_cluster, file.size, ascii_font_table);
    char str_buf[1024];
    sprintf(str_buf,
            "Build time: %s, resolution: (%u, %u), kernel_size: %u\n\n"
            "kb fAKe",
            __TIMESTAMP__, vbe_info->width, vbe_info->height,
            boot_meta->kernel_size);
    write_string_at(40, 40, str_buf, 0xffffff, 0x0);

    init_mouse();
    io_out8(PIC1_DATA, 0xF9);  // 允许键盘中断和从中断器
    io_out8(PIC2_DATA, 0xEF);  // 允许鼠标中断
    uint8_t mouse_stage = 0;
    uint8_t mouse_data[3];
    while (true) {
        asm("cli");
        if (keyboard_mouse::keyboard_buffer.len != 0) {
            uint8_t keycode = keyboard_mouse::keyboard_buffer.get();
            asm("sti");
            char buf[128];
            sprintf(buf, "You pressed: 0x%02x", keycode);
            write_string_at(0, 0, buf, 0x0, 0xFFFFFF);
        } else if (keyboard_mouse::mouse_buffer.len != 0) {
            uint8_t data = keyboard_mouse::mouse_buffer.get();
            asm("sti");
            if (data == 0xFA && mouse_stage == 0) {
                mouse_stage = 1;
            } else if (mouse_stage == 1 || mouse_stage == 2) {
                mouse_data[mouse_stage - 1] = data;
                mouse_stage++;
            } else if (mouse_stage == 3) {
                mouse_data[2] = data;
                mouse_stage = 1;
                char buf[128];
                sprintf(buf, "You moved: 0x%02x 0x%02x 0x%02x", mouse_data[0],
                        mouse_data[1], mouse_data[2]);
                write_string_at(0, 18, buf, 0x0, 0xFFFFFF);
            }
        } else {
            asm("sti;hlt");
        }
    }
}

void write_segment_entry(void* ptr,
                         uint32_t base,
                         uint32_t limit,
                         uint8_t flags,
                         uint8_t access_byte) {
    uint64_t u = 0;
    u = u | (limit & 0xffff);
    u = u | ((((uint64_t)limit) >> 16) & 0xf) << 48;

    u = u | (((uint64_t)base & 0xffffff) << 16);
    u = u | ((((uint64_t)base >> 24) & 0xff) << 56);

    u = u | (((uint64_t)access_byte) << 40);

    u = u | (((uint64_t)flags & 0x0f) << 52);

    *((uint64_t*)ptr) = u;
}

void write_interrupt_entry(void* ptr,
                           uint32_t offset,
                           uint16_t selector,
                           uint8_t present,
                           uint8_t DPL,
                           uint8_t S,
                           uint8_t gate_type) {
    uint64_t u = 0;
    // 写offset
    u |= (offset & 0xffff);
    u |= (((uint64_t)offset >> 16) << 48);
    // 写selector
    u |= (selector << 16);
    // 拼接type并写进去
    uint8_t type = 0;
    type |= ((present & 1) << 7);
    type |= ((DPL & 3) << 5);
    type |= ((S & 1) << 4);
    type |= (gate_type & 0x0f);
    u |= ((uint64_t)type << 40);
    *((uint64_t*)ptr) = u;
}