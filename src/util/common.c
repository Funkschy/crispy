#include "common.h"

uint64_t next_pow_of_2(uint64_t num) {
    uint64_t n = num - 1;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    // if n is 0 add 1
    n += 1 + (n == 0);
    return n;
}