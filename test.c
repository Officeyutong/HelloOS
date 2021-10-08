#include "mydef.h"

int main() {
    long long v;
    write_segment_entry(&v, 0x00280000, 0x7ffff, 0x04, 0x9a);

    return 0;
}