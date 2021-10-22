#include "../include/serial.h"
#include "../include/kutil.h"
using namespace serial;
int SerialDevice::init() {
    using namespace Port;
    io_out8(base + 1, 0);
    // 设置比特率
    io_out8(base + 3, 0x80);
    io_out8(base + 0, divisor & 0xFF);
    io_out8(base + 1, (divisor >> 8) & 0xFF);
    io_out8(base + 3, 0x03);
    io_out8(base + 2, 0xC7);  // FIFO

    io_out8(base + 4, 0x0B);
    io_out8(base + 4, 0x1E);
    //测试
    io_out8(base + 0, 0xAE);
    if (io_in8(base + 0) != 0xAE) {
        return false;
    }

    io_out8(base + 4, 0x0F);
    return true;
}
uint8_t SerialDevice::receive() {
    while (!has_received_data())
        ;
    return io_in8(base);
}
void SerialDevice::send(uint8_t byte) {
    while (!is_transmit_empty())
        ;
    io_out8(base, byte);
}

bool SerialDevice::has_received_data() {
    return io_in8(base + Port::LINE_STATUS) & 1;
}
bool SerialDevice::is_transmit_empty() {
    return !!(io_in8(base + Port::LINE_STATUS) & 0x20);
}

void SerialDevice::write_str(const char* s) {
    while (*s)
        send(*(s++));
}