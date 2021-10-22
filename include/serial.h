#ifndef _SERIAL_H
#define _SERIAL_H
#include <cinttypes>

namespace serial {
namespace Port {
const uint8_t DATA = 0x0;
const uint8_t INTERRUPT_ENABLED = 0x1;
const uint8_t INTERRUPT_IDENTIFICATION = 0x2;
const uint8_t LINE_CONTROL = 0x3;
const uint8_t MODEM_CONTROL = 0x4;
const uint8_t LINE_STATUS = 0x5;
const uint8_t MODEM_STATUS = 0x6;
const uint8_t SCRATCH = 0x7;

const uint8_t BAUD_HIGH = 0x01;
const uint8_t BAUD_LOW = 0x00;

};  // namespace Port
namespace COM {
const uint16_t COM1 = 0x3F8;
const uint16_t COM2 = 0x2F8;
const uint16_t COM3 = 0x3E8;
const uint16_t COM4 = 0x2E8;

};  // namespace COM
struct SerialDevice {
    uint16_t base;
    uint32_t divisor;
    SerialDevice(uint16_t device, uint32_t divisor = 3)
        : base(device), divisor(divisor) {}
    int init();
    uint8_t receive();
    void send(uint8_t byte);
    bool has_received_data();
    bool is_transmit_empty();
    void write_str(const char* s);
};
};  // namespace serial

#endif