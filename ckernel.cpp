#include "ckernel.h"
#include <inttypes.h>
#include "include/asmfunc.h"
#include "include/display.h"
#include "include/kprintf.h"
#include "include/string.h"

static PageTable* next_page_table = kernel_page_table_first;
static const FAT12BootSector* boot_sector_ref = boot_sector;
static int val = 0xcafebabe;
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
    write_segment_entry(gdt_info + 1, 0, 0xfffff, 0x9a, 0x0c);  // 全局可执行
    write_segment_entry(gdt_info + 2, 0, 0xfffff, 0x9a, 0x0c);  // 全局不可执行
    load_gdt(3 * 8 - 1, gdt_info);

    for (int i = 0; i < IDT_COUNT; i++)
        write_interrupt_entry(idt_info + i, 0, 0, 0, 0, 0, 0);
    load_idt(8 * IDT_COUNT - 1, idt_info);
}
// 处理begin~end的内核内存页映射
static void map_memory_page(uint32_t begin, uint32_t end) {
    for (uint32_t i = begin; i <= end; i++) {
        uint32_t dir_index = i / 1024;
        uint32_t table_index = i % 1024;
        auto& entry = kernel_page_directory->entries[dir_index];
        if (!entry.present) {
            PageTable* table_addr = next_page_table--;
            entry.pagetable_addr = ((uint32_t)table_addr) / 4096;
            entry.cache_disabled = 1;
            entry.present = 1;
            entry.read_or_wrte = 1;
            entry.zerobit = 0;
            entry.read_or_wrte = 1;
            entry.cache_disabled = 1;
            entry.user_or_supervisor = 0;
        }
        auto& table_ref = *((PageTable*)(entry.pagetable_addr * 4096));
        auto& table_entry_ref = table_ref.entries[table_index];
        table_entry_ref.addr = i;
        table_entry_ref.cache_disabled = 1;
        table_entry_ref.global = 0;
        table_entry_ref.present = 1;
        table_entry_ref.read_or_wrte = 1;
        table_entry_ref.user_or_supervisor = 0;
        table_entry_ref.zerobit = 0;
    }
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
                    // 从右到左压栈
                    write_pixel_at(i, j, color);
                }
            }
        }
    }
}
int cnt = 0;
extern "C" void kernel_main() {
    // boot_sector_ref =;
    // const FAT12BootSector& boot_sector_ref = *boot_sector;
    init_gdt_idt();
    cnt++;
    init_paging();
    cnt++;
    paint_screen();
    cnt++;
    while (true) {
        __asm__("hlt");
    }
    // // 设置段表
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