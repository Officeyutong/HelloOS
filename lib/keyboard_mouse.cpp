#include "../include/keyboard_mouse.h"
#include "../include/kutil.h"
// using namespace keyboard;
namespace keyboard_mouse {

BufferQueue<32> keyboard_buffer;
BufferQueue<128> mouse_buffer;

void wait_for_keyboard_send_ready() {
    while (io_in8(KEYBOARD_COMMAND) & (0x02))
        ;
}
}  // namespace keyboard
