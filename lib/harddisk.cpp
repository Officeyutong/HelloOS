#include "../include/harddisk.h"
#include <algorithm>
#include "../include/kutil.h"
#include "../include/string.h"
#define lengthof(arr) (sizeof(arr) / sizeof(arr[0]))

using namespace fat32;
ATAPIO_FAT32Reader::ATAPIO_FAT32Reader(
    const FAT32BootSector* info,
    const boot_meta_info_struct* boot_meta_info)
    : info(info), boot_meta_info(boot_meta_info) {}

IdentityResult ATAPIO_FAT32Reader::get_identity_data(void* buf) {
    this->reset();
    io_out8(ATAPort::DRIVE_SELECT, ATADrive::MASTER);
    for (int i = 0x1F2; i <= 0x1F5; i++)
        io_out8(i, 0);
    io_out8(ATAPort::COMMAND, 0xEC);
    if (io_in8(ATAPort::STATUS) == 0)
        return IdentityResult::NOT_ATA_DEVICE;
    while (io_in8(ATAPort::STATUS) & ATAStatus::BSY)
        ;
    if (io_in8(ATAPort::LBAMid) != 0 || io_in8(ATAPort::LBAHigh) != 0)
        return IdentityResult::NOT_EXISTS;
    while (true) {
        auto r = io_in8(ATAPort::STATUS);
        using namespace ATAStatus;
        if (r & ERR)
            return IdentityResult::ERROR;
        if (r & DRQ)
            break;
    }
    io_ins16(ATAPort::DATA, (uint32_t)buf, 256);
    return IdentityResult::DONE;
}
void ATAPIO_FAT32Reader::read_sector(void* buffer,
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
    if (!(io_in8(ATAPort::STATUS) & ATAStatus::RDY)) {
        wait_for_DRQ();
    }
    for (int i = 0; i < sector_count; i++) {
        wait_for_BSY();
        wait_for_DRQ();
        io_ins16(0x1F0, ((uint32_t)buffer) + i * 512, 256);
        delay_400ns();
    }
}
uint32_t ATAPIO_FAT32Reader::find_file(uint32_t cluster,
                                       const char* filename_to_find,
                                       DirectoryEntry& output) {
    char longfilename[256];
    char* tail = longfilename;
    int long_fileentry_count = 0;
    while (cluster < 0x0FFFFFF8) {
        //多读了一个扇区进来，啥玩意
        DirectoryEntry buf[256];
        // memset(buf, 0xff, sizeof(buf));
        auto curr_sector = this->get_sector_by_cluster(cluster);
        this->read_sector(buf, curr_sector & 0xFFFFFFFF,
                          (curr_sector >> 32) & 0xFFFF, info->mbr.cluster_size);
        for (int i = 0; i < 256; i++) {
            const DirectoryEntry& fileentry = buf[i];
            if (buf[i].attr == 0x0F) {  // 长文件名
                const LongFilenameEntry& filename =
                    *((LongFilenameEntry*)&buf[i]);
                // 现在暂时只支持ASCII文件名
                for (uint32_t j = 0; j < lengthof(filename.name_first5); j++)
                    *(tail++) = filename.name_first5[j] & 0xff;
                for (uint32_t j = 0; j < lengthof(filename.name_next6); j++)
                    *(tail++) = filename.name_next6[j] & 0xff;
                for (uint32_t j = 0; j < lengthof(filename.name_final2); j++)
                    *(tail++) = filename.name_final2[j] & 0xff;
                long_fileentry_count++;
                continue;
            } else if (buf[i].attr == 0) {
                break;
            } else {
                if (tail == longfilename) {
                    // 不使用长文件名
                    char local_filename[12];
                    char* tail = local_filename;
                    for (uint32_t i = 0; i < lengthof(fileentry.filename);
                         i++) {
                        if (fileentry.filename[i] != ' ')
                            *(tail++) = fileentry.filename[i];
                    }
                    *(tail++) = '.';
                    for (uint32_t i = 0; i < lengthof(fileentry.ext); i++) {
                        if (fileentry.ext[i] != ' ')
                            *(tail++) = fileentry.ext[i];
                    }
                    *(tail++) = '\0';
                    if (strcmp(filename_to_find, local_filename) == 0) {
                        output = fileentry;
                        return ((uint32_t)fileentry.cluster_high2byte << 16) |
                               fileentry.cluster_low2byte;
                    }
                } else {
                    // *(tail++) = '\0';
                    struct LongFilenameEntryPack {
                        char name[13];
                    } __attribute__((packed, aligned(1)));
                    LongFilenameEntryPack* start =
                        (LongFilenameEntryPack*)longfilename;
                    std::reverse(start, start + long_fileentry_count);
                    tail = longfilename;
                    long_fileentry_count = 0;
                    if (strcmp(longfilename, filename_to_find) == 0) {
                        output = fileentry;
                        return ((uint32_t)fileentry.cluster_high2byte << 16) |
                               fileentry.cluster_low2byte;
                    }
                }
            }
        }
        cluster = this->read_cluster_num(cluster);
    }
    return FILE_NOT_EXISTS;
}

uint32_t ATAPIO_FAT32Reader::read_cluster_num(uint32_t cluster) {
    uint64_t offset = info->mbr.fat_start_sector + (uint64_t)(cluster / 128);
    uint32_t buf[128];
    read_sector(buf, offset & 0xFFFFFFFF, (offset >> 32) & 0xFFFF, 1);
    return buf[cluster % 128];
}

void ATAPIO_FAT32Reader::read_file(uint32_t cluster,
                                   uint32_t size,
                                   void* buffer) {
    char* buf = (char*)buffer;
    char localbuf[8192];
    uint32_t cluster_size_in_bytes = 512 * info->mbr.cluster_size;
    while (cluster < 0x0FFFFFF8) {
        auto sector = this->get_sector_by_cluster(cluster);
        this->read_sector(localbuf, sector & 0xFFFFFFFF,
                          (sector >> 32) & 0xFFFF, info->mbr.cluster_size);
        if (size >= cluster_size_in_bytes) {
            memcpy(buf, localbuf, cluster_size_in_bytes);
            size -= cluster_size_in_bytes;
            buf += cluster_size_in_bytes;
            cluster = this->read_cluster_num(cluster);
        } else {
            memcpy(buf, localbuf, size);
            return;
        }
    }
}