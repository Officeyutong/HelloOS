#include <cinttypes>
void write_segment_entry(void* ptr,
                         uint32_t base,
                         uint32_t limit,
                         uint8_t flags,
                         uint8_t access_byte) {
    uint64_t u = 0;
    u = u | (limit & 0xffff);
    u = u | ((((uint64_t)limit) >> 16) & 0xf) << 48;

    u = u | (((uint64_t)base & 0xffffff) << 16);
    u = u | ((((uint64_t)base >> 24) & 0xff) << 56);

    u = u | (((uint64_t)access_byte) << 40);

    u = u | (((uint64_t)flags & 0x0f) << 52);

    *((uint64_t*)ptr) = u;
}