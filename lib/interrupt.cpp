#include <cinttypes>
#include "../ckernel.h"
#include "../include/display.h"
#include "../include/keyboard_mouse.h"
#include "../include/kprintf.h"
#include "../include/kutil.h"
extern "C" void c_interrupt_0x21(void* esp) {
    io_out8(PIC1_COMMAND, 0x61);
    uint8_t keycode = io_in8(keyboard_mouse::KEYBOARD_DATA);
    keyboard_mouse::keyboard_buffer.put(keycode);
}

extern "C" void c_interrupt_0x27(void* esp) {
    io_out8(PIC1_COMMAND, 0x67);
}
extern "C" void c_interrupt_0x2c(void* esp) {
    // write_string_at(0, 18, "You moved mouse!", 0x0, 0xffffff);
    io_out8(PIC2_COMMAND, 0x64);
    io_out8(PIC1_COMMAND, 0x62);
    uint8_t data = io_in8(keyboard_mouse::KEYBOARD_DATA);
    keyboard_mouse::mouse_buffer.put(data);
}
