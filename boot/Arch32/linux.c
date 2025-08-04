/*
 * BloodHorn Bootloader
 *
 * This file is part of BloodHorn and is licensed under the MIT License.
 * See the root of the repository for license details.
 */
#include <stdint.h>
#include "compat.h"
#include <string.h>
#include "linux.h"

extern void read_sector(uint32_t lba, uint8_t* buf);
extern void* allocate_memory(uint32_t size);

struct linux_kernel_header {
    uint8_t setup_sects;
    uint16_t root_flags;
    uint32_t syssize;
    uint16_t ram_size;
    uint16_t vid_mode;
    uint16_t root_dev;
    uint16_t boot_flag;
    uint16_t jump;
    uint32_t header;
    uint16_t version;
    uint32_t realmode_swtch;
    uint16_t start_sys_seg;
    uint16_t kernel_version;
    uint8_t type_of_loader;
    uint8_t loadflags;
    uint16_t setup_move_size;
    uint32_t code32_start;
    uint32_t ramdisk_image;
    uint32_t ramdisk_size;
    uint32_t bootsect_kludge;
    uint16_t heap_end_ptr;
    uint8_t ext_loader_ver;
    uint8_t ext_loader_type;
    uint32_t cmd_line_ptr;
    uint32_t initrd_addr_max;
    uint32_t kernel_alignment;
    uint8_t relocatable_kernel;
    uint8_t min_alignment;
    uint16_t xloadflags;
    uint32_t cmdline_size;
    uint32_t hardware_subarch;
    uint64_t hardware_subarch_data;
    uint32_t payload_offset;
    uint32_t payload_length;
    uint64_t setup_data;
    uint64_t pref_address;
    uint32_t init_size;
    uint32_t handover_offset;
};

int linux_load_kernel(const char* kernel_path, const char* initrd_path, const char* cmdline) {
    uint8_t* kernel_data = NULL;
    uint32_t kernel_size = 0;
    
    // Load kernel file
    if (load_file(kernel_path, &kernel_data, &kernel_size) != 0) {
        return -1;
    }
    
    // Parse kernel header
    struct linux_kernel_header* header = (struct linux_kernel_header*)kernel_data;
    
    // Verify it's a Linux kernel
    if (header->header != 0x53726448) { // "HdrS" magic
        return -1;
    }
    
    // Calculate setup size
    uint32_t setup_size = (header->setup_sects + 1) * 512;
    if (setup_size == 0) setup_size = 4 * 512; // Default 4 sectors
    
    // Load kernel to 0x100000 (1MB)
    uint8_t* kernel_dest = (uint8_t*)0x100000;
    memcpy(kernel_dest, kernel_data + setup_size, kernel_size - setup_size);
    
    // Load initrd if specified
    uint32_t initrd_addr = 0;
    uint32_t initrd_size = 0;
    if (initrd_path && strlen(initrd_path) > 0) {
        uint8_t* initrd_data = NULL;
        if (load_file(initrd_path, &initrd_data, &initrd_size) == 0) {
            // Place initrd after kernel
            initrd_addr = 0x100000 + kernel_size - setup_size;
            memcpy((void*)initrd_addr, initrd_data, initrd_size);
        }
    }
    
    // Setup boot parameters
    struct linux_boot_params* params = (struct linux_boot_params*)0x90000;
    memset(params, 0, sizeof(struct linux_boot_params));
    
    // Copy setup code
    memcpy(params, kernel_data, setup_size);
    
    // Set command line
    if (cmdline && strlen(cmdline) > 0) {
        strcpy((char*)0x90000 + 0x0020, cmdline);
        params->cmd_line_ptr = 0x90020;
    }
    
    // Set initrd parameters
    if (initrd_addr > 0) {
        params->ramdisk_image = initrd_addr;
        params->ramdisk_size = initrd_size;
    }
    
    // Set memory parameters
    params->mem_upper = 0x7FE00; // 640KB - 1MB
    params->ext_mem_k = 0x7FE00; // Extended memory in KB
    
    // Jump to kernel
    void (*kernel_entry)(struct linux_boot_params*) = (void*)0x100000;
    kernel_entry(params);
    
    return 0;
}

int linux_verify_kernel(const char* kernel_path) {
    uint8_t* kernel_data = NULL;
    uint32_t kernel_size = 0;
    
    if (load_file(kernel_path, &kernel_data, &kernel_size) != 0) {
        return -1;
    }
    
    struct linux_kernel_header* header = (struct linux_kernel_header*)kernel_data;
    
    // Check magic number
    if (header->header != 0x53726448) {
        return -1;
    }
    
    // Check version
    if (header->version < 0x0200) { // Require 2.0+
        return -1;
    }
    
    return 0;
} 

int boot_linux_kernel(uint8_t* kernel_data, uint32_t kernel_size, uint8_t* initrd_data, uint32_t initrd_size, const char* cmdline) {
    struct linux_kernel_header* header = (struct linux_kernel_header*)kernel_data;
    
    if (header->header != 0x53726448) {
        return -1;
    }
    
    uint32_t setup_size = (header->setup_sects + 1) * 512;
    if (setup_size == 0) setup_size = 4 * 512;
    
    uint8_t* kernel_dest = (uint8_t*)0x100000;
    memcpy(kernel_dest, kernel_data + setup_size, kernel_size - setup_size);
    
    uint32_t initrd_addr = 0;
    if (initrd_data && initrd_size > 0) {
        initrd_addr = 0x100000 + kernel_size - setup_size;
        memcpy((void*)initrd_addr, initrd_data, initrd_size);
    }
    
    struct linux_boot_params* params = (struct linux_boot_params*)0x90000;
    memset(params, 0, sizeof(struct linux_boot_params));
    
    memcpy(params, kernel_data, setup_size);
    
    if (cmdline && strlen(cmdline) > 0) {
        strcpy((char*)0x90000 + 0x0020, cmdline);
        params->cmd_line_ptr = 0x90020;
    }
    
    if (initrd_addr > 0) {
        params->ramdisk_image = initrd_addr;
        params->ramdisk_size = initrd_size;
    }
    
    params->mem_upper = 0x7FE00;
    params->ext_mem_k = 0x7FE00;
    
    void (*kernel_entry)(struct linux_boot_params*) = (void*)0x100000;
    kernel_entry(params);
    
    return 0;
} 