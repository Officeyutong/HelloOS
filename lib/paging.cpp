#include "../include/paging.h"
#include "../ckernel.h"

PageAllocator* page_allocator = (PageAllocator*)0x00100000;

// 处理begin~end的内核内存页映射
void map_reserve_memory_page(uint32_t begin, uint32_t end) {
    for (uint32_t i = begin; i <= end; i++) {
        page_allocator->set_usable(i, false);
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
bool PageAllocator::is_allocated(uint32_t page) {
    return !!(allocated[page / 8] & (1 << (page % 8)));
}
bool PageAllocator::is_usable(uint32_t page) {
    return !!(usable[page / 8] & (1 << (page % 8)));
}
uint32_t PageAllocator::allocate(bool& ok) {
    if (allocated_pages == usable_pages) {
        ok = false;
        return 0;
    }
    uint32_t i = last_alloc + 1;
    uint32_t loopcnt = 0;
    while (i != last_alloc) {
        loopcnt++;
        if (is_usable(i) && !is_allocated(i)) {
            allocated[i / 8] |= (1 << (i % 8));
            last_alloc = i;
            ok = true;
            allocated_pages++;
            return i;
        }
        i++;
        if (i >= TOTAL_PAGES)
            i %= TOTAL_PAGES;
        if (i == last_alloc)
            break;
    }
    ok = false;
    return 0;
}
bool PageAllocator::free(uint32_t page) {
    if (!is_allocated(page) || !is_usable(page)) {
        return false;
    }
    allocated[page / 8] &= (~(1 << (page % 8)));
    allocated_pages--;
    return true;
}
void PageAllocator::set_usable(uint32_t page, bool usable) {
    auto& ref = this->usable[page / 8];
    if (usable) {
        ref |= (1 << (page % 8));
    } else {
        ref &= (~(1 << (page % 8)));
    }
}

uint32_t PageAllocator::count_usable_pages() {
    uint32_t count = 0;
    for (auto v : usable)
        count += __builtin_popcount(v);
    this->usable_pages = count;
    return count;
}
uint32_t PageAllocator::count_allocated_pages() {
    uint32_t count = 0;
    for (auto v : allocated)
        count += __builtin_popcount(v & 0xFF);
    this->allocated_pages = count;
    return count;
}