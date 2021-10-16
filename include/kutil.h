#ifndef _KUTIL_H
#define _KUTIL_H
#include <cinttypes>
extern "C" {
void io_out8(uint16_t port, uint8_t val);
void io_out16(uint16_t port, uint16_t val);
uint16_t io_in16(uint16_t port);
uint8_t io_in8(uint16_t port);
void io_ins16(uint16_t port, uint32_t buffer, uint32_t count);
void io_sti();
void io_stihlt();
void io_cli();
}
#endif