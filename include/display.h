#ifndef _DISPLAY_H
#define _DISPLAY_H
#include <cinttypes>
void write_pixel_at(int x, int y, uint32_t pix);
void write_char_at(int x,
                   int y,
                   char chr,
                   uint32_t color,
                   int32_t background = -1);
void write_string_at(int x,
                     int y,
                     const char* str,
                     uint32_t color,
                     int32_t background = -1);
#endif
