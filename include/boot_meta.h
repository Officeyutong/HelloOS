#ifndef _BOOT_META_INCLUDE
#define _BOOT_META_INCLUDE
#include <cinttypes>
struct boot_meta_info_struct {
    uint16_t message_prefix;
    uint32_t kernel_size;
    uint32_t last_file_size;
    uint8_t led_state;
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
    uint16_t magic_fat12;
} __attribute__((packed, aligned(1)));

#endif