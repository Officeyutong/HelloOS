#ifndef _ASCII_FONT_H
#define _ASCII_FONT_H

struct ASCIIFontTable {
    // 每个字符用16个8bit整数来描述
    // 即16行8列的点阵，从上到下
    // 每个char按从低位到高位，从左到右
    char data[256][16];
};

#endif