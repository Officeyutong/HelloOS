#include <cinttypes>
void write_interrupt_entry(void* ptr,
                           uint32_t offset,
                           uint16_t selector,
                           uint8_t present,
                           uint8_t DPL,
                           uint8_t S,
                           uint8_t gate_type) {
    uint64_t u = 0;
    // 写offset
    u |= (offset & 0xffff);
    u |= (((uint64_t)offset >> 16) << 48);
    // 写selector
    u |= (selector << 16);
    // 拼接type并写进去
    uint8_t type = 0;
    type |= ((present & 1) << 7);
    type |= ((DPL & 3) << 5);
    type |= ((S & 1) << 4);
    type |= (gate_type & 0x0f);
    u |= ((uint64_t)type << 40);
    *((uint64_t*)ptr) = u;
}