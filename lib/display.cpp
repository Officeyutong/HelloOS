#include <cinttypes>
#include "../ckernel.h"
#include "../include/display.h"
uint32_t transform_color(uint32_t src) {
    //转换RGB到本地颜色
    uint32_t red = (src >> 16) & 0xff;
    uint32_t green = (src >> 8) & 0xff;
    uint32_t blue = (src)&0xff;
    uint32_t ret = 0;
    ret |= ((red & ((1 << vbe_info->red_mask) - 1)) << vbe_info->red_position);
    ret |= ((green & ((1 << vbe_info->green_mask) - 1))
            << vbe_info->green_position);
    ret |=
        ((blue & ((1 << vbe_info->blue_mask) - 1)) << vbe_info->blue_position);
    return ret;
}

/*
显存通常是0xfd00 0000
*/
void write_pixel_at(int x, int y, uint32_t pix) {
    uint32_t* offset =
        (uint32_t*)(y * vbe_info->pitch + x * (vbe_info->bpp / 8) +
                    (uint32_t)vbe_info->framebuffer);
    *offset = transform_color(pix);
}
// 以某个位置为左上角，输出一个宽8高16的点阵文字
void write_char_at(int x, int y, char chr, uint32_t color, int32_t background) {
    const auto& chardata = ascii_font_table->data[(uint32_t)chr];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            int b = !!(chardata[j] & (1 << i));
            if (b)
                write_pixel_at(x + i, y + j, color);
            else if (background != -1)
                write_pixel_at(x + i, y + j, background);
        }
    }
}
void write_string_at(int x,
                     int y,
                     const char* str,
                     uint32_t color,
                     int32_t background) {
    int x_offset = 0, y_offset = 0;
    while (*str) {
        auto chr = *str;
        if (chr == '\n') {
            x_offset = 0;
            y_offset++;
            str++;
            continue;
        }
        write_char_at(x + x_offset * 8, y + y_offset * 16, chr, color,
                      background);
        x_offset++;
        str++;
    }
}