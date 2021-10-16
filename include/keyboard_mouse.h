#ifndef _KEYBOARD_H
#define _KEYBOARD_H
#include <cinttypes>
#include "buffer_queue.h"
namespace keyboard_mouse {
// const uint32_t  = 32;

const uint16_t KEYBOARD_DATA = 0x60;
const uint16_t KEYBOARD_COMMAND = 0x64;
void wait_for_keyboard_send_ready();

extern BufferQueue<32> keyboard_buffer;
extern BufferQueue<128> mouse_buffer;
}  // namespace keyboard

#endif