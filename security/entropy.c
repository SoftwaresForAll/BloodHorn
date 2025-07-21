#include "entropy.h"
#include <stdint.h>
uint32_t entropy_get(void) {
    uint32_t t;
    asm volatile ("rdtsc" : "=a"(t));
    return t ^ (uintptr_t)&t;
} 