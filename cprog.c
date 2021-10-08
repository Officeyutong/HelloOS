#include <stdbool.h>
#include "asmfunc.h"
#include "mydef.h"
/*
显存通常是0xfd00 0000
*/
void write_pixel_at(int x, int y, uint32 pix) {
    uint32* offset = (uint32*)(y * vbe_info->pitch + x * (vbe_info->bpp / 8) +
                               vbe_info->framebuffer);
    *offset = pix;
}

void MyMain() {
    for (int i = 0; i < vbe_info->width; i++) {
        for (int j = 0; j < vbe_info->height; j++) {
            write_pixel_at(i, j, 0xff0000);
        }
    }
    // const int SECTION_COUNT = 32;
    // int sectionHeight = vbe_info->height / SECTION_COUNT;
    // int seccolors[] = {0xffff00, 0xff00ff, 0x00ffff};
    // for (int a = 0; a < SECTION_COUNT; a++) {
    //     for (int k = 0; k < SECTION_COUNT; k++) {
    //         for (int i = a * vbe_info->width / SECTION_COUNT;
    //              i < (a + 1) * vbe_info->width / SECTION_COUNT; i++) {
    //             for (int j = k * sectionHeight; j < (k + 1) * sectionHeight;
    //                  j++) {
    //                 // 从右到左压栈
    //                 write_pixel_at(i, j, seccolors[(a + k) % 3]);
    //             }
    //         }
    //     }
    // }

    while (true) {
        io_halt();
    }
}

void write_segment_entry(void* ptr,
                         uint32 base,
                         uint32 limit,
                         uint8 flags,
                         uint8 access_byte) {
    uint64 u = 0;
    u = u | (limit & 0xffff);
    u = u | ((((uint64)limit) >> 16) & 0xf) << 48;

    u = u | (((uint64)base & 0xffffff) << 16);
    u = u | ((((uint64)base >> 24) & 0xff) << 56);

    u = u | (((uint64)access_byte) << 40);

    u = u | (((uint64)flags & 0x0f) << 52);

    *((uint64*)ptr) = u;
}
