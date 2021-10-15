#ifndef _HARDDISK_H
#define _HARDDISK_H
#include <cinttypes>
#include "../include/boot_meta.h"
namespace fat32 {
const uint32_t FILE_NOT_EXISTS = 0x0FFFFFFF;

struct FATTimeField {
    uint16_t seconds : 5;
    uint16_t minutes : 6;
    uint16_t hour : 5;
} __attribute__((packed, aligned(1)));

struct FATDateField {
    uint16_t day : 5;
    uint16_t month : 4;
    uint16_t year : 7;
} __attribute__((packed, aligned(1)));

struct DirectoryEntry {
    char filename[8];
    char ext[3];
    uint8_t attr;
    uint8_t reserve;
    uint8_t creation_time_tenths_of_second;
    FATTimeField creation_time;
    FATDateField creation_date;
    FATDateField last_accessed_date;
    uint16_t cluster_high2byte;
    FATTimeField last_modification_time;
    FATDateField last_modification_date;
    uint16_t cluster_low2byte;
    uint32_t size;
} __attribute__((packed, aligned(1)));

struct LongFilenameEntry {
    uint8_t order;
    uint16_t name_first5[5];
    uint8_t attr;
    uint8_t long_entry_type;
    uint8_t checksum;
    uint16_t name_next6[6];
    uint16_t zero;
    uint16_t name_final2[2];
} __attribute__((packed, aligned(1)));

struct FAT32Reader {
    const FAT32BootSector* info;
    const boot_meta_info_struct* boot_meta_info;
    FAT32Reader(const FAT32BootSector* info,
                const boot_meta_info_struct* boot_meta_info);
    void read_sector(void* buffer,
                     uint32_t low,
                     uint16_t high = 0,
                     uint16_t sector_count = 1);
    uint32_t find_file(uint32_t cluster,
                       const char* filename,
                       DirectoryEntry& output);
    uint32_t read_cluster_num(uint32_t cluster);
    void read_file(uint32_t cluster, uint32_t size, void* buffer);
    uint64_t get_sector_by_cluster(uint32_t cluster) {
        return this->boot_meta_info->rootdir_start_sector +
               (uint64_t)(cluster - 2) * info->mbr.cluster_size;
    }
};
}  // namespace fat32
#endif