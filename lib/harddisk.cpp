#include "../include/harddisk.h"
#include "../include/kutil.h"
void FAT32Reader::read_sector(void* buffer,
                              uint32_t low,
                              uint16_t high,
                              uint16_t sector_count) {
    // asm("outb 0x40, 0x1F6");
    io_out8(0x1F6, 0x40);

    // asm("outb %al, 0x1F2" ::"al"(sector_count >> 8));
    io_out8(0x1F2, sector_count >> 8);

    // asm("outb %al, 0x1F3" ::"al"((low >> 24) & 0xFF));
    io_out8(0x1F3, (low >> 24) & 0xFF);

    // asm("outb %al, 0x1F4" ::"al"(high & 0xFF));
    io_out8(0x1F4, (high)&0xFF);

    // asm("outb %al, 0x1F4" ::"al"((high >> 8) & 0xFF));
    io_out8(0x1F5, (high >> 8) & 0xFF);

    // asm("outb %al, 0x1F2" ::"al"(sector_count & 0xFF));
    io_out8(0x1F2, sector_count & 0xFF);

    // asm("outb %al, 0x1F3" ::"al"(low & 0xFF));
    io_out8(0x1F3, low & 0xFF);

    // asm("outb %al, 0x1F4" ::"al"((low >> 8) & 0xFF));
    io_out8(0x1F4, (low >> 8) & 0xFF);

    // asm("outb %al, 0x1F5" ::"al"((low >> 16) & 0xFF));
    io_out8(0x1F5, (low >> 16) & 0xFF);

    // asm("outb %al, 0x1F4" ::"al"(high & 0xFF));
    io_out8(0x1F7, 0x24);

    io_ins16(0x1F0, (uint32_t)buffer, sector_count * 512);
}