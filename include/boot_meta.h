#ifndef _BOOT_META_INCLUDE
#define _BOOT_META_INCLUDE
#include <cinttypes>
struct boot_meta_info_struct {
    uint16_t message_prefix;
    uint32_t kernel_size;
    uint32_t last_file_size;
    uint8_t led_state;
} __attribute__((packed, aligned(1)));

#endif