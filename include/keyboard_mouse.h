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

struct MouseDecoder {
    uint8_t stage = 0;
    uint8_t buf[3];
    int32_t x = 0, y = 0;
    uint8_t button;
    bool sendByte(uint8_t data) {
        if (stage == 0 && data == 0xfa) {
            stage = 1;
            return false;
        } else if (stage == 1) {
            if ((data & 0xc8) == 0x08) {
                buf[0] = data;
                stage = 2;
            }
            return false;
        } else if (stage == 2) {
            buf[1] = data;
            stage = 3;
            return false;
        } else {
            buf[2] = data;
            stage = 1;
            this->button = buf[0] & 0x07;
            this->x = buf[1], this->y = buf[2];
            if (buf[0] & 0x10)
                x |= 0xFFFFFF00;
            if (buf[0] & 0x20)
                y |= 0xFFFFFF00;
            y *= -1;
            return true;
        }
    }
};

}  // namespace keyboard_mouse

#endif