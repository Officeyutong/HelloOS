#ifndef _PAGING_H
#define _PAGING_H
#include <cinttypes>
#include "../include/string.h"
const uint32_t TOTAL_PAGES = 1 << 20;
struct PageDirectoryEntry {
    uint32_t present : 1;
    uint32_t read_or_wrte : 1;
    uint32_t user_or_supervisor : 1;
    uint32_t write_through : 1;
    uint32_t cache_disabled : 1;
    uint32_t accessed : 1;
    uint32_t zerobit : 1;
    uint32_t size : 1;
    uint32_t ignored : 1;
    uint32_t avail : 3;
    uint32_t pagetable_addr : 20;
} __attribute__((packed, aligned(1)));

struct PageTableEntry {
    uint32_t present : 1;
    uint32_t read_or_wrte : 1;
    uint32_t user_or_supervisor : 1;
    uint32_t write_through : 1;
    uint32_t cache_disabled : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t zerobit : 1;
    uint32_t global : 1;
    uint32_t avail : 3;
    uint32_t addr : 20;
} __attribute__((packed, aligned(1)));

struct PageDirectory {
    PageDirectoryEntry entries[1024];
};
struct PageTable {
    PageTableEntry entries[1024];
};
struct PageAllocator {
    uint8_t allocated[1 << 17];
    uint8_t usable[1 << 17];
    uint32_t last_alloc;
    uint32_t usable_pages;
    uint32_t allocated_pages;
    uint32_t allocate(bool& ok);
    bool free(uint32_t page);
    bool is_allocated(uint32_t page);
    bool is_usable(uint32_t page);
    void set_usable(uint32_t page, bool usable);
    uint32_t count_usable_pages();
    uint32_t count_allocated_pages();
    void init() {
        memset(allocated, 0, sizeof allocated);
        memset(usable, 0, sizeof usable);
        last_alloc = usable_pages = allocated_pages = 0;
    }
} __attribute__((packed, aligned(1)));

void map_reserve_memory_page(uint32_t begin, uint32_t end);
extern PageAllocator* page_allocator;
#endif