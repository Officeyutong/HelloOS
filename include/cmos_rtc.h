#include <cinttypes>
#include "../include/kutil.h"
namespace cmos_rtc {

const uint16_t CMOS_INPUT_PORT = 0x70;
const uint16_t CMOS_DATA_PORT = 0x71;

namespace CMOSRegister {
const uint16_t seconds = 0x00;
const uint16_t minutes = 0x02;
const uint16_t hours = 0x04;
const uint16_t weekday = 0x06;
const uint16_t day_of_month = 0x07;
const uint16_t month = 0x08;
const uint16_t year = 0x09;
const uint16_t century = 0x32;

const uint8_t STATUS_B = 0x0B;
};  // namespace CMOSRegister

struct CMOSResponse {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t weekday;
    uint8_t day_of_month;
    uint8_t month;
    // 0~99
    uint8_t year;
    uint8_t century;
    void print_str(char* buf);
};

struct CMOS_RTC {
    CMOSResponse read();
    bool update_in_progress() {
        io_out8(CMOS_INPUT_PORT, 0x0A);
        return !!(io_in8(CMOS_DATA_PORT) & 0x80);
    }
};
}  // namespace cmos_rtc