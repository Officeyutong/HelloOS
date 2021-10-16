#ifndef _MEMORY_H
#define _MEMORY_H
#include <cinttypes>
struct MemoryUsageEntry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t ext;
} __attribute__((packed, aligned(1)));

struct MemoryUsagePack {
    uint32_t count;
    MemoryUsageEntry arr[16];
} __attribute__((packed, aligned(1)));

#endif