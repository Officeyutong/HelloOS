#ifndef _HARDDISK_H
#define _HARDDISK_H
#include <cinttypes>
struct FAT32Reader {
    void read_sector(void* buffer,
                     uint32_t low,
                     uint16_t high = 0,
                     uint16_t sector_count = 1);
};

#endif