#ifndef _BUFFER_QUEUE_H
#define _BUFFER_QUEUE_H
#include <cinttypes>
template <uint32_t BUFFER_SIZE = 32, typename Element = uint8_t>
struct BufferQueue {
    Element data[BUFFER_SIZE];
    int head, tail;
    int len;
    void put(Element c) {
        if (len == BUFFER_SIZE)
            return;
        int next = ((tail + 1)) % BUFFER_SIZE;
        data[tail] = c;
        tail = next;
        len++;
    }
    int32_t get() {
        if (len == 0)
            return -1;
        int ret = data[head];
        head = (head + 1) % BUFFER_SIZE;
        len--;
        return ret;
    }
    void init() { head = tail = len = 0; }
};

#endif