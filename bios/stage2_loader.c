// BloodHorn BIOS Stage 2 Loader
// Loads the Multiboot2 loader and passes control to it. 

#include <stdint.h>

#define MULTIBOOT2_LOADER_ADDR 0x9000
#define MULTIBOOT2_LOADER_SECTOR 3
#define MULTIBOOT2_LOADER_SIZE 16*1024 // 16KB

extern void bios_read_sectors(uint8_t drive, uint32_t lba, uint16_t count, void* buf);
typedef void (*multiboot2_loader_entry_t)(uint8_t boot_drive);

void stage2_main(uint8_t boot_drive) {
    // Load Multiboot2 loader
    bios_read_sectors(boot_drive, MULTIBOOT2_LOADER_SECTOR, 32, (void*)MULTIBOOT2_LOADER_ADDR);
    // Jump to Multiboot2 loader
    multiboot2_loader_entry_t entry = (multiboot2_loader_entry_t)MULTIBOOT2_LOADER_ADDR;
    entry(boot_drive);
} 