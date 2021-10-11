#ifndef _PAGING_H
#define _PAGING_H
#include <cinttypes>
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
#endif