#ifndef _BOOT_META_INCLUDE
#define _BOOT_META_INCLUDE
#include <cinttypes>
struct boot_meta_info_struct {
    uint16_t message_prefix;
    uint32_t kernel_size;
    uint32_t last_file_size;
    uint8_t led_state;
    uint32_t fat_lenght;
    uint16_t fat_start_sector;
    uint32_t rootdir_start_sector;
    uint16_t rootdir_sector_count;
    uint8_t cluster_size_in_sector;
    uint32_t cluster_size_in_byte;
    uint32_t data_start_sector;
    uint32_t rootdir_entry_count;
} __attribute__((packed, aligned(1)));
struct MBRBootSector {
    uint8_t jmp[2];
    uint8_t magic;
    char oem_name[8];
    uint16_t sector_size;
    uint8_t cluster_size;
    uint16_t fat_start_sector;
    uint8_t fat_count;
    uint16_t root_entry_count;
    uint16_t sector_count;
    uint8_t type;
    uint16_t fat_length;
    uint16_t sector_per_track;
    uint16_t head_count;
    uint32_t start_lba;
    uint32_t sector_count_32b;
} __attribute__((packed, aligned(1)));
struct FAT12BootSector {
    MBRBootSector mbr;
    uint8_t int13_num;
    uint8_t _preserve;
    uint8_t signature;
    uint32_t serial;
    char label[11];
    char type_name[8];
    char code[448];
    uint16_t magic_fat;
} __attribute__((packed, aligned(1)));

struct FAT32BootSector {
    MBRBootSector mbr;
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t fat_version_num;
    uint32_t root_cluster;
    uint16_t fsinfo_sector;
    uint16_t bootsector_backup;
    char reserve1[12];
    uint8_t int13_drivenumber;
    uint8_t reserve2;
    uint8_t signature;
    uint32_t serial;
    char label[11];
    char type_name[8];
    char code[420];
    uint16_t magic_fat;
} __attribute__((packed, aligned(1)));

#endif