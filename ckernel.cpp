#include "ckernel.h"
#include <inttypes.h>
#include "include/algo.h"
#include "include/cmos_rtc.h"
#include "include/display.h"
#include "include/gdt.h"
#include "include/harddisk.h"
#include "include/idt.h"
#include "include/interrupt.h"
#include "include/keyboard_mouse.h"
#include "include/kprintf.h"
#include "include/kutil.h"
#include "include/memory.h"
#include "include/serial.h"
#include "include/string.h"

vbe_mode_info_structure* vbe_info = (vbe_mode_info_structure*)0xE600;
boot_meta_info_struct* boot_meta = (boot_meta_info_struct*)0x0600;
FAT32BootSector* boot_sector = (FAT32BootSector*)0x7C00;
GDT* gdt_info = (GDT*)0x270000;
IDT* idt_info = (IDT*)0x26F800;

PageDirectory* kernel_page_directory = (PageDirectory*)0x26D000;
PageTable* next_kernel_page_table = (PageTable*)0x26C000;
ASCIIFontTable* ascii_font_table = (ASCIIFontTable*)0x26E000;
MemoryUsagePack* memory_usage_pack = (MemoryUsagePack*)0x700;

serial::SerialDevice* com1;
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
static void collect_memory() {
    uint64_t total_memory_in_bytes = 0;
    char buf[512];
    algo::sort(memory_usage_pack->arr,
               memory_usage_pack->arr + memory_usage_pack->count,
               [](const MemoryUsageEntry& a, const MemoryUsageEntry& b) {
                   return a.base < b.base;
               });
    for (uint32_t i = 0; i < memory_usage_pack->count; i++) {
        const auto& curr = memory_usage_pack->arr[i];
        sprintf(buf, "base=%08llx, length=%08llx, type=%u", curr.base,
                curr.length, curr.type);
        write_string_at(40, 300 + i * 18, buf, 0xffffff, 0);
        if (curr.type == 1)
            total_memory_in_bytes += curr.length;
    }
    sprintf(buf, "Total memory: %lldMB", total_memory_in_bytes / 1024 / 1024);
    write_string_at(40, 280, buf, 0xFFFFFF, 0);
}
static void init_gdt_idt() {
    for (uint32_t i = 0; i < GDT_COUNT; i++)
        write_segment_entry(gdt_info + i, 0, 0, 0, 0);
    // write_segment_entry(gdt_info , 0, 0, 0, 0);                  // 0项
    write_segment_entry(gdt_info + 1, 0, 0xfffff, 0x0c, 0x9a);  // 全局可执行
    write_segment_entry(gdt_info + 2, 0, 0xfffff, 0x0c, 0x92);  // 全局不可执行
    load_gdt(8 * GDT_COUNT - 1, gdt_info);

    for (uint32_t i = 0; i < IDT_COUNT; i++)
        write_interrupt_entry(idt_info + i, 0, 0, 0, 0, 0, 0);
    // 页错误
    write_interrupt_entry(idt_info + 0x0e, (uint32_t)&asm_interrupt_0x0e,
                          1 << 3, 1, 0, 0, 0x0e);
    // 键盘
    write_interrupt_entry(idt_info + 0x21, (uint32_t)&asm_interrupt_0x21,
                          1 << 3, 1, 0, 0, 0x0e);
    write_interrupt_entry(idt_info + 0x27, (uint32_t)&asm_interrupt_0x27,
                          1 << 3, 1, 0, 0, 0x0e);
    // 鼠标
    write_interrupt_entry(idt_info + 0x2c, (uint32_t)&asm_interrupt_0x2c,
                          1 << 3, 1, 0, 0, 0x0e);

    load_idt(8 * IDT_COUNT - 1, idt_info);
}

static void init_paging() {
    page_allocator->init();
    // 标记可用内存区
    for (uint32_t i = 0; i < memory_usage_pack->count; i++) {
        const auto& curr = memory_usage_pack->arr[i];
        if (curr.type == 1) {
            uint64_t addr = curr.base;
            if (addr % 4096 != 0)
                addr = (addr / 4096) + 1;
            else
                addr /= 4096;
            addr *= 4096;
            uint64_t tail_addr = curr.base + curr.length - 1;
            for (; addr + 4096 - 1 <= tail_addr; addr += 4096) {
                page_allocator->set_usable(addr / 4096, true);
            }
        }
    }

    map_reserve_memory_page(0, 0xff);
    map_reserve_memory_page(0x100, 0x480);
    const uint32_t framebuffer_size = vbe_info->pitch * vbe_info->height;
    const uint32_t tailaddr =
        ((uint32_t)vbe_info->framebuffer) + framebuffer_size - 1;
    map_reserve_memory_page(((uint32_t)vbe_info->framebuffer) / 4096,
                            1 + tailaddr / 4096);
    page_allocator->count_usable_pages();
    page_allocator->count_allocated_pages();
    // for (uint32_t i = 0; i < 4096; i++) {
    //     auto& entry = kernel_page_directory->entries[i];
    //     // 重新映射页表，释放之前的空间
    //     if (entry.present) {
    //         bool ok;
    //         auto page_id = page_allocator->allocate(ok);
    //         memcpy((void*)(page_id * 4096),
    //                (void*)(entry.pagetable_addr * 4096), 4096);
    //         entry.pagetable_addr = page_id;
    //     }
    // }
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
    ATAPIO_FAT32Reader reader(boot_sector, boot_meta);
    DirectoryEntry file;
    auto start_cluster =
        reader.find_file(reader.info->root_cluster, "ascii_font.bin", file);
    reader.read_file(start_cluster, file.size, ascii_font_table);
    char str_buf[1024];
    char boot_time[512];
    cmos_rtc::CMOS_RTC().read().print_str(boot_time);
    serial::SerialDevice com1(serial::COM::COM1);
    ::com1 = &com1;
    bool serial_status = com1.init();
    sprintf(str_buf,
            "Build time: %s, boot time: %s \nResolution: (%u, %u), "
            "kernel_size: %u\n\nUsable pages: %u, usable memory: %u "
            "MB\nSerial status: %d",
            __TIMESTAMP__, boot_time, vbe_info->width, vbe_info->height,
            boot_meta->kernel_size, page_allocator->usable_pages,
            page_allocator->usable_pages * 4096 / 1024 / 1024, serial_status);
    com1.write_str("orz kernelbin\r\n");
    write_string_at(40, 40, str_buf, 0xffffff, 0x0);
    bool ok;
    auto u = page_allocator->allocate(ok);
    {
        sprintf(
            str_buf,
            "Allocated page id: 0x%x, allocated page count %u, query state: "
            "%d, %d",
            u, page_allocator->allocated_pages, page_allocator->is_allocated(u),
            page_allocator->is_allocated(u + 1));
        write_string_at(40, 120, str_buf, 0xffffff, 0x0);
    }
    {
        bool ok1 = page_allocator->free(u);
        bool ok2 = page_allocator->free(u);

        sprintf(str_buf, "Freed  %x, ok: %d, double free: %d", u, ok1, ok2);
        write_string_at(40, 140, str_buf, 0xffffff, 0x0);
    }

    collect_memory();
    init_mouse();
    io_out8(PIC1_DATA, 0xF9);  // 允许键盘中断和从中断器
    io_out8(PIC2_DATA, 0xEF);  // 允许鼠标中断
    keyboard_mouse::MouseDecoder mouse;
    int32_t mouse_x = 0, mouse_y = 0;
    // *((int*)0x1141234) = 233;
    // for (int* v = (int*)0x480000; v <= (int*)0x1140000; v += 4096 / 4) {
    //     *v = 2333;
    // }
    while (true) {
        asm("cli");
        if (keyboard_mouse::keyboard_buffer.len != 0) {
            uint8_t keycode = keyboard_mouse::keyboard_buffer.get();
            asm("sti");
            char buf[128];
            sprintf(buf, "You pressed: 0x%02x", keycode);
            write_string_at(0, 0, buf, 0xFFFFFF, 0x0);
        } else if (keyboard_mouse::mouse_buffer.len != 0) {
            uint8_t data = keyboard_mouse::mouse_buffer.get();
            asm("sti");
            if (mouse.sendByte(data)) {
                mouse_x += mouse.x, mouse_y += mouse.y;
                char buf[128];
                sprintf(buf, "Button: %x, x: %4d, y: %4d", mouse.button,
                        mouse_x, mouse_y);
                write_string_at(0, 18, buf, 0xFFFFFF, 0);
            }
        } else {
            asm("sti;hlt");
        }
    }
}
