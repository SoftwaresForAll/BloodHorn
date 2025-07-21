#ifndef BLOODHORN_MULTIBOOT2_H
#define BLOODHORN_MULTIBOOT2_H

#include <stdint.h>

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot_tag_basic_meminfo {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
};

#endif // BLOODHORN_MULTIBOOT2_H 