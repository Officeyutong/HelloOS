#ifndef _BOOT_META_INCLUDE
#define _BOOT_META_INCLUDE
struct boot_meta_info_struct {
    uint16 message_prefix;
    uint32 kernel_size;
    uint32 last_file_size;
    uint8 led_state;
} __attribute__((packed, aligned(1)));

#endif