#include "../include/cmos_rtc.h"
#include "../include/kprintf.h"

static void hex_to_dec(uint8_t& v) {
    v = (v / 16) * 10 + v % 16;
}

namespace cmos_rtc {
CMOSResponse CMOS_RTC::read() {
    CMOSResponse resp;
    const auto& read = [](uint16_t reg) {
        io_out8(CMOS_INPUT_PORT, reg);
        // for (int i = 0; i < 10; i++)
        //     io_in8(CMOS_DATA_PORT);
        return io_in8(CMOS_DATA_PORT);
    };
    using namespace CMOSRegister;
    while (update_in_progress())
        ;
    resp.century = read(century);
    resp.day_of_month = read(day_of_month);
    resp.hours = read(hours);
    resp.minutes = read(minutes);
    resp.seconds = read(seconds);
    resp.weekday = read(weekday);
    resp.year = read(year);
    resp.month = read(month);
    uint8_t status_b = read(STATUS_B);
    bool enable_24h = !!(status_b & (1 << 1));
    bool enable_binary_mode = !!(status_b & (1 << 2));
    bool afternoon = (!enable_24h) && (resp.hours & 0x80);
    resp.hours &= (~0x80);
    if (!enable_binary_mode) {
        hex_to_dec(resp.century);
        hex_to_dec(resp.day_of_month);
        hex_to_dec(resp.hours);
        hex_to_dec(resp.minutes);
        hex_to_dec(resp.seconds);
        hex_to_dec(resp.weekday);
        hex_to_dec(resp.year);
        hex_to_dec(resp.month);
    }
    if (afternoon)
        resp.hours += 12;
    return resp;
}
void CMOSResponse::print_str(char* buf) {
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            (int32_t)year + (int32_t)century * 100, (int32_t)month,
            (int32_t)day_of_month, (int32_t)hours, (int32_t)minutes,
            (int32_t)seconds);
}

}  // namespace cmos_rtc