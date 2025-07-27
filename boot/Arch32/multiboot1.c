#include <stdint.h>
#include "compat.h"
#include <string.h>
#include "multiboot1.h"

extern void* allocate_memory(uint32_t size);
extern int load_file(const char* path, uint8_t** data, uint32_t* size);

struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
};

struct multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
};

struct multiboot_module {
    uint32_t mod_start;
    uint32_t mod_end;
    uint32_t string;
    uint32_t reserved;
};

int multiboot1_load_kernel(const char* kernel_path, const char* cmdline) {
    uint8_t* kernel_data = NULL;
    uint32_t kernel_size = 0;
    
    // Load kernel file
    if (load_file(kernel_path, &kernel_data, &kernel_size) != 0) {
        return -1;
    }
    
    // Parse Multiboot 1 header
    uint32_t* header = (uint32_t*)kernel_data;
    
    // Verify Multiboot 1 magic
    if (header[0] != MULTIBOOT_HEADER_MAGIC) {
        return -1;
    }
    
    // Find header address
    uint32_t header_addr = 0;
    uint32_t load_addr = 0;
    uint32_t load_end_addr = 0;
    uint32_t bss_end_addr = 0;
    uint32_t entry_addr = 0;
    
    // Parse header fields
    for (uint32_t i = 1; i < 8; i++) {
        if (header[i] == MULTIBOOT_HEADER_TAG_LOAD) {
            load_addr = header[i + 1];
            load_end_addr = header[i + 2];
            bss_end_addr = header[i + 3];
        } else if (header[i] == MULTIBOOT_HEADER_TAG_ENTRY) {
            entry_addr = header[i + 1];
        }
    }
    
    // Default values if not specified
    if (load_addr == 0) load_addr = 0x100000; // 1MB
    if (entry_addr == 0) entry_addr = load_addr;
    
    // Load kernel to specified address
    uint32_t load_size = load_end_addr - load_addr;
    if (load_size > kernel_size) load_size = kernel_size;
    
    memcpy((void*)load_addr, kernel_data, load_size);
    
    // Zero out BSS
    if (bss_end_addr > load_end_addr) {
        memset((void*)load_end_addr, 0, bss_end_addr - load_end_addr);
    }
    
    // Setup Multiboot 1 info structure
    struct multiboot_info* mb_info = (struct multiboot_info*)0x90000;
    memset(mb_info, 0, sizeof(struct multiboot_info));
    
    // Set flags
    mb_info->flags = MULTIBOOT_INFO_MEMORY | MULTIBOOT_INFO_CMDLINE | MULTIBOOT_INFO_BOOTDEV;
    
    // Set memory info
    mb_info->mem_lower = 640; // 640KB
    mb_info->mem_upper = 0x7FE00; // Extended memory in KB
    
    // Set command line
    if (cmdline && strlen(cmdline) > 0) {
        char* cmdline_addr = (char*)0x90020;
        strcpy(cmdline_addr, cmdline);
        mb_info->cmdline = (uint32_t)cmdline_addr;
    }
    
    // Set boot device
    mb_info->boot_device = 0x8000; // First hard disk, first partition
    
    // Setup memory map
    struct multiboot_mmap_entry* mmap = (struct multiboot_mmap_entry*)0x90100;
    mmap[0].size = sizeof(struct multiboot_mmap_entry) - 4;
    mmap[0].addr = 0;
    mmap[0].len = 0x100000; // 1MB
    mmap[0].type = 1; // Available memory
    
    mmap[1].size = sizeof(struct multiboot_mmap_entry) - 4;
    mmap[1].addr = 0x100000;
    mmap[1].len = 0x7FF00000; // ~2GB
    mmap[1].type = 1; // Available memory
    
    mb_info->mmap_length = 2 * sizeof(struct multiboot_mmap_entry);
    mb_info->mmap_addr = (uint32_t)mmap;
    
    // Jump to kernel
    void (*kernel_entry)(uint32_t, struct multiboot_info*) = (void*)entry_addr;
    kernel_entry(MULTIBOOT_BOOTLOADER_MAGIC, mb_info);
    
    return 0;
}

int multiboot1_verify_kernel(const char* kernel_path) {
    uint8_t* kernel_data = NULL;
    uint32_t kernel_size = 0;
    
    if (load_file(kernel_path, &kernel_data, &kernel_size) != 0) {
        return -1;
    }
    
    uint32_t* header = (uint32_t*)kernel_data;
    
    // Check Multiboot 1 magic
    if (header[0] != MULTIBOOT_HEADER_MAGIC) {
        return -1;
    }
    
    // Check header checksum
    uint32_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += header[i];
    }
    if (checksum != 0) {
        return -1;
    }
    
    return 0;
}

int multiboot1_load_module(const char* module_path, const char* cmdline) {
    uint8_t* module_data = NULL;
    uint32_t module_size = 0;
    
    if (load_file(module_path, &module_data, &module_size) != 0) {
        return -1;
    }
    
    // Load module to high memory
    uint32_t module_addr = 0x200000; // 2MB
    memcpy((void*)module_addr, module_data, module_size);
    
    // Add to module list
    struct multiboot_module* modules = (struct multiboot_module*)0x90200;
    static int module_count = 0;
    
    modules[module_count].mod_start = module_addr;
    modules[module_count].mod_end = module_addr + module_size;
    modules[module_count].string = 0x90300 + module_count * 64;
    
    // Copy module command line
    if (cmdline && strlen(cmdline) > 0) {
        char* cmdline_addr = (char*)modules[module_count].string;
        strcpy(cmdline_addr, cmdline);
    }
    
    module_count++;
    
    return 0;
} 

int boot_multiboot1_kernel(uint8_t* kernel_data, uint32_t kernel_size, const char* cmdline) {
    struct multiboot_info* info = (struct multiboot_info*)0x1000;
    memset(info, 0, sizeof(struct multiboot_info));
    
    info->flags = MULTIBOOT_INFO_MEMORY | MULTIBOOT_INFO_CMDLINE;
    info->mem_lower = 640;
    info->mem_upper = 0x7FE00;
    
    if (cmdline && strlen(cmdline) > 0) {
        strcpy((char*)0x2000, cmdline);
        info->cmdline = 0x2000;
    }
    
    uint32_t kernel_entry = 0x100000;
    memcpy((void*)kernel_entry, kernel_data, kernel_size);
    
    void (*entry_point)(uint32_t, uint32_t) = (void*)kernel_entry;
    entry_point(MULTIBOOT_BOOTLOADER_MAGIC, (uint32_t)info);
    
    return 0;
} 