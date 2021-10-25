#include <cinttypes>
#include "../ckernel.h"
#include "../include/display.h"
#include "../include/keyboard_mouse.h"
#include "../include/kprintf.h"
#include "../include/kutil.h"
#include "../include/paging.h"
extern "C" void c_interrupt_0x0e(void* addr,
                                 PageDirectory* page_dir,
                                 uint32_t error) {
    char buf[512];
    sprintf(buf, "Page fault! Addr:0x%08x, page dir: 0x%08p, error: 0x%08x\r\n",
            addr, page_dir, error);
    com1->write_str(buf);
    // while(true);
    if (page_dir == kernel_page_directory) {  // 内核页表
        uint32_t page = uint32_t(addr) / 4096;
        bool ok;
        uint32_t phy_page = page_allocator->allocate(ok);
        sprintf(buf,
                "Allocated kernel page: phy_page: 0x%08x, virtual_page: "
                "0x%08x, allocate state: %d\r\n",
                phy_page, page, ok);
        com1->write_str(buf);
        if (!ok) {
            while (true)
                ;
        } else {
            map_kernel_page(page, phy_page);
        }
    } else {
        while (true)
            ;
    }
}
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