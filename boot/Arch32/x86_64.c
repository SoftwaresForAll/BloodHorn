#include <stdint.h>
#include "compat.h"
#include <string.h>
#include "x86_64.h"

extern void* allocate_memory(uint32_t size);
extern int load_file(const char* path, uint8_t** data, uint32_t* size);

struct x86_64_boot_params {
    uint64_t mem_lower;
    uint64_t mem_upper;
    uint64_t boot_device;
    uint64_t cmdline;
    uint64_t mods_count;
    uint64_t mods_addr;
    uint64_t syms[4];
    uint64_t mmap_length;
    uint64_t mmap_addr;
    uint64_t drives_length;
    uint64_t drives_addr;
    uint64_t config_table;
    uint64_t boot_loader_name;
    uint64_t apm_table;
    uint64_t vbe_control_info;
    uint64_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
};

int x86_64_load_kernel(const char* kernel_path, const char* initrd_path, const char* cmdline) {
    uint8_t* kernel_data = NULL;
    uint64_t kernel_size = 0;
    
    if (load_file(kernel_path, &kernel_data, (uint32_t*)&kernel_size) != 0) {
        return -1;
    }
    
    uint32_t* header = (uint32_t*)kernel_data;
    
    if (header[0] == 0x53726448) {
        return x86_64_boot_linux(kernel_data, kernel_size, initrd_path, cmdline);
    } else if (header[0] == 0x1BADB002) {
        return x86_64_boot_multiboot1(kernel_data, kernel_size, cmdline);
    } else if (header[0] == 0xE85250D6) {
        return x86_64_boot_multiboot2(kernel_data, kernel_size, cmdline);
    }
    
    return -1;
}

int x86_64_boot_linux(uint8_t* kernel_data, uint64_t kernel_size, const char* initrd_path, const char* cmdline) {
    struct linux_kernel_header* header = (struct linux_kernel_header*)kernel_data;
    
    uint32_t setup_size = (header->setup_sects + 1) * 512;
    if (setup_size == 0) setup_size = 4 * 512;
    
    uint8_t* kernel_dest = (uint8_t*)0x100000;
    memcpy(kernel_dest, kernel_data + setup_size, kernel_size - setup_size);
    
    uint64_t initrd_addr = 0;
    uint64_t initrd_size = 0;
    if (initrd_path && strlen(initrd_path) > 0) {
        uint8_t* initrd_data = NULL;
        uint32_t initrd_size32 = 0;
        if (load_file(initrd_path, &initrd_data, &initrd_size32) == 0) {
            initrd_size = initrd_size32;
            initrd_addr = 0x100000 + kernel_size - setup_size;
            memcpy((void*)initrd_addr, initrd_data, initrd_size);
        }
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

int x86_64_boot_multiboot1(uint8_t* kernel_data, uint64_t kernel_size, const char* cmdline) {
    struct x86_64_boot_params* info = (struct x86_64_boot_params*)0x1000;
    memset(info, 0, sizeof(struct x86_64_boot_params));
    
    info->mem_lower = 640;
    info->mem_upper = 0x7FE00;
    info->flags = 0x00000001 | 0x00000004;
    
    if (cmdline && strlen(cmdline) > 0) {
        strcpy((char*)0x2000, cmdline);
        info->cmdline = 0x2000;
    }
    
    uint64_t kernel_entry = 0x100000;
    memcpy((void*)kernel_entry, kernel_data, kernel_size);
    
    void (*entry_point)(uint32_t, uint64_t) = (void*)kernel_entry;
    entry_point(0x2BADB002, (uint64_t)info);
    
    return 0;
}

int x86_64_boot_multiboot2(uint8_t* kernel_data, uint64_t kernel_size, const char* cmdline) {
    struct multiboot2_info* info = (struct multiboot2_info*)0x1000;
    uint8_t* tag_ptr = (uint8_t*)info + 8;
    
    info->total_size = 8;
    info->reserved = 0;
    
    struct multiboot2_tag_basic_meminfo* meminfo = (struct multiboot2_tag_basic_meminfo*)tag_ptr;
    meminfo->type = 4;
    meminfo->size = 16;
    meminfo->mem_lower = 640;
    meminfo->mem_upper = 0x7FE00;
    tag_ptr += meminfo->size;
    info->total_size += meminfo->size;
    
    if (cmdline && strlen(cmdline) > 0) {
        struct multiboot2_tag_string* cmdline_tag = (struct multiboot2_tag_string*)tag_ptr;
        cmdline_tag->type = 1;
        strcpy(cmdline_tag->string, cmdline);
        cmdline_tag->size = 8 + strlen(cmdline) + 1;
        tag_ptr += cmdline_tag->size;
        info->total_size += cmdline_tag->size;
    }
    
    struct multiboot2_tag* end_tag = (struct multiboot2_tag*)tag_ptr;
    end_tag->type = 0;
    end_tag->size = 8;
    info->total_size += 8;
    
    uint64_t kernel_entry = 0x100000;
    memcpy((void*)kernel_entry, kernel_data, kernel_size);
    
    void (*entry_point)(uint32_t, uint64_t) = (void*)kernel_entry;
    entry_point(0x36d76289, (uint64_t)info);
    
    return 0;
}

int x86_64_verify_kernel(const char* kernel_path) {
    uint8_t* kernel_data = NULL;
    uint32_t kernel_size = 0;
    
    if (load_file(kernel_path, &kernel_data, &kernel_size) != 0) {
        return -1;
    }
    
    uint32_t* header = (uint32_t*)kernel_data;
    
    if (header[0] == 0x53726448 || header[0] == 0x1BADB002 || header[0] == 0xE85250D6) {
        return 0;
    }
    
    return -1;
} 