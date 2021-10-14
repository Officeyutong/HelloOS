#include "../include/paging.h"
#include "../ckernel.h"
// 处理begin~end的内核内存页映射
void map_memory_page(uint32_t begin, uint32_t end) {
    for (uint32_t i = begin; i <= end; i++) {
        uint32_t dir_index = i / 1024;
        uint32_t table_index = i % 1024;
        auto& entry = kernel_page_directory->entries[dir_index];
        if (!entry.present) {
            PageTable* table_addr = next_kernel_page_table--;
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