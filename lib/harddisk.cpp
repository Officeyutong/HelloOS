#include "../include/harddisk.h"
#include "../include/kutil.h"
#include "../include/string.h"
using namespace fat32;
FAT32Reader::FAT32Reader(const FAT32BootSector* info,
                         const boot_meta_info_struct* boot_meta_info)
    : info(info), boot_meta_info(boot_meta_info) {}

void FAT32Reader::read_sector(void* buffer,
                              uint32_t low,
                              uint16_t high,
                              uint16_t sector_count) {
    io_out8(0x1F6, 0x40);
    io_out8(0x1F2, sector_count >> 8);
    io_out8(0x1F3, (low >> 24) & 0xFF);
    io_out8(0x1F4, (high)&0xFF);
    io_out8(0x1F5, (high >> 8) & 0xFF);
    io_out8(0x1F2, sector_count & 0xFF);
    io_out8(0x1F3, low & 0xFF);
    io_out8(0x1F4, (low >> 8) & 0xFF);
    io_out8(0x1F5, (low >> 16) & 0xFF);
    io_out8(0x1F7, 0x24);

    for (int i = 0; i < sector_count; i++) {
        for (int j = 0; j < 14; j++)
            io_in8(0x1F7);
        while (io_in8(0x1F7) & (1 << 7))
            ;
        io_ins16(0x1F0, ((uint32_t)buffer) + i * 512, 256);
    }
}
uint32_t FAT32Reader::find_file(uint32_t cluster,
                                const char* filename,
                                DirectoryEntry& output) {
    while (cluster < 0x0FFFFFF8) {
        //多读了一个扇区进来，啥玩意
        DirectoryEntry buf[256];
        // memset(buf, 0xff, sizeof(buf));
        auto curr_sector = this->boot_meta_info->rootdir_start_sector +
                           (uint64_t)(cluster - 2) * info->mbr.cluster_size;
        this->read_sector(buf, curr_sector & 0xFFFFFFFF,
                          (curr_sector >> 32) & 0xFFFF, info->mbr.cluster_size);
        cluster = this->read_cluster_num(cluster);
    }
    return FILE_NOT_EXISTS;
}

uint32_t FAT32Reader::read_cluster_num(uint32_t cluster) {
    uint64_t offset = info->mbr.fat_start_sector + (uint64_t)(cluster / 128);
    uint32_t buf[128];
    read_sector(buf, offset & 0xFFFFFFFF, (offset >> 32) & 0xFFFF, 1);
    return buf[cluster % 128];
}