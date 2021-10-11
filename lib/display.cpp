#include <cinttypes>
#include "../ckernel.h"

/*
显存通常是0xfd00 0000
*/
void write_pixel_at(int x, int y, uint32_t pix) {
    uint32_t* offset =
        (uint32_t*)(y * vbe_info->pitch + x * (vbe_info->bpp / 8) +
                    vbe_info->framebuffer);
    *offset = pix;
}