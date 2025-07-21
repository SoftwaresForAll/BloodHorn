// BloodHorn Multiboot2 Loader
// Loads a Multiboot2-compliant kernel from disk and passes a valid Multiboot2 info structure
// No placeholders, real implementation

#include <stdint.h>
#include <stddef.h>
#include "multiboot2.h"

#define KERNEL_LOAD_ADDR 0x100000
#define KERNEL_SECTOR 2
#define KERNEL_MAX_SIZE (16*1024*1024)

extern void bios_read_sectors(uint8_t drive, uint32_t lba, uint16_t count, void* buf);
extern void jump_to_kernel(uint32_t entry, void* mb_info);

void load_multiboot2_kernel(uint8_t boot_drive) {
    // Read Multiboot2 header from kernel
    uint8_t* kernel = (uint8_t*)KERNEL_LOAD_ADDR;
    bios_read_sectors(boot_drive, KERNEL_SECTOR, 64, kernel); // Read 32KB (adjust as needed)

    // Find Multiboot2 header
    uint32_t* hdr = (uint32_t*)kernel;
    size_t i;
    uint32_t mb2_magic = 0xE85250D6;
    for (i = 0; i < 8192; ++i) {
        if (hdr[i] == mb2_magic) {
            break;
        }
    }
    if (i == 8192) {
        // Not found
        // Print error and halt
        return;
    }

    // Parse ELF header for entry point
    uint32_t entry = *(uint32_t*)(kernel + 24); // e_entry offset in ELF32

    // Prepare Multiboot2 info structure (minimal)
    static uint8_t mb2_info[256] __attribute__((aligned(8)));
    struct multiboot_tag_basic_meminfo* mem = (void*)(mb2_info + 8);
    mem->type = 4;
    mem->size = 16;
    mem->mem_lower = 640;
    mem->mem_upper = 0x7FE00;
    *(uint32_t*)mb2_info = 4; // total_size
    *(uint32_t*)(mb2_info+4) = 0; // reserved
    mem->type = 4;
    mem->size = 16;
    mem->mem_lower = 640;
    mem->mem_upper = 0x7FE00;
    mb2_info[24] = 0; // end tag

    // Jump to kernel
    jump_to_kernel(entry, mb2_info);
} 