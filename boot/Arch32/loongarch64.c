/*
 * BloodHorn Bootloader
 *
 * This file is part of BloodHorn and is licensed under the MIT License.
 * See the root of the repository for license details.
 */
#include <stdint.h>
#include "compat.h"
#include <string.h>
#include "loongarch64.h"

extern void* allocate_memory(uint32_t size);
extern int load_file(const char* path, uint8_t** data, uint32_t* size);

struct loongarch64_boot_params {
    uint64_t dtb_addr;
    uint64_t initrd_addr;
    uint64_t initrd_size;
    uint64_t cmdline_addr;
    uint64_t cmdline_size;
    uint64_t kernel_addr;
    uint64_t kernel_size;
    uint64_t mem_start;
    uint64_t mem_size;
    uint64_t acpi_rsdp;
    uint64_t efi_systab;
};

struct loongarch64_linux_header {
    uint32_t code0;
    uint32_t code1;
    uint64_t text_offset;
    uint64_t image_size;
    uint64_t flags;
    uint32_t version;
    uint32_t res1;
    uint64_t res2;
    uint64_t res3;
    uint64_t res4;
    uint32_t magic;
    uint32_t hdr_offset;
    uint32_t hdr_size;
    uint32_t end_offset;
    uint32_t end_size;
    uint64_t hdr_version;
    uint64_t comp_version;
    uint64_t name_offset;
    uint64_t name_size;
    uint64_t id_offset;
    uint64_t id_size;
    uint64_t hdr_string_offset;
    uint64_t hdr_string_size;
};

int loongarch64_load_kernel(const char* kernel_path, const char* initrd_path, const char* cmdline) {
    uint8_t* kernel_data = NULL;
    uint64_t kernel_size = 0;
    
    if (load_file(kernel_path, &kernel_data, (uint32_t*)&kernel_size) != 0) {
        return -1;
    }
    
    struct loongarch64_linux_header* header = (struct loongarch64_linux_header*)kernel_data;
    
    if (header->magic == 0x4c4f4f4e) {
        return loongarch64_boot_linux(kernel_data, kernel_size, initrd_path, cmdline);
    }
    
    return -1;
}

int loongarch64_boot_linux(uint8_t* kernel_data, uint64_t kernel_size, const char* initrd_path, const char* cmdline) {
    struct loongarch64_linux_header* header = (struct loongarch64_linux_header*)kernel_data;
    
    uint64_t kernel_load_addr = 0x9000000000200000;
    uint64_t dtb_addr = 0x9000000000300000;
    uint64_t initrd_addr = 0;
    uint64_t initrd_size = 0;
    
    if (initrd_path && strlen(initrd_path) > 0) {
        uint8_t* initrd_data = NULL;
        uint32_t initrd_size32 = 0;
        if (load_file(initrd_path, &initrd_data, &initrd_size32) == 0) {
            initrd_size = initrd_size32;
            initrd_addr = dtb_addr + 0x100000;
            if (initrd_data && initrd_size > 0) {
                memcpy((void*)initrd_addr, initrd_data, initrd_size);
            }
        }
    }

    uint64_t cmdline_addr = 0;
    uint64_t cmdline_size = 0;
    if (cmdline && strlen(cmdline) > 0) {
        cmdline_addr = initrd_addr + initrd_size + 0x1000;
        cmdline_size = strlen(cmdline) + 1;
        // Use strncpy for buffer safety; assumes cmdline_addr points to a buffer of at least cmdline_size
        strncpy((char*)cmdline_addr, cmdline, cmdline_size - 1);
        ((char*)cmdline_addr)[cmdline_size - 1] = '\0';
    }

    if (kernel_data && kernel_size > 0) {
        memcpy((void*)kernel_load_addr, kernel_data, kernel_size);
    }

    struct loongarch64_boot_params* params = (struct loongarch64_boot_params*)0x9000000000100000;
    memset(params, 0, sizeof(struct loongarch64_boot_params));
    
    params->dtb_addr = dtb_addr;
    params->initrd_addr = initrd_addr;
    params->initrd_size = initrd_size;
    params->cmdline_addr = cmdline_addr;
    params->cmdline_size = cmdline_size;
    params->kernel_addr = kernel_load_addr;
    params->kernel_size = kernel_size;
    params->mem_start = 0x9000000000000000;
    params->mem_size = 0x800000000;
    params->acpi_rsdp = 0;
    params->efi_systab = 0;
    
    uint64_t entry_point = kernel_load_addr + header->text_offset;
    void (*kernel_entry)(uint64_t, uint64_t, uint64_t) = (void*)entry_point;
    kernel_entry(0, dtb_addr, (uint64_t)params);
    
    return 0;
}

int loongarch64_boot_uefi(uint8_t* kernel_data, uint64_t kernel_size, const char* cmdline) {
    uint64_t kernel_load_addr = 0x9000000000200000;
    uint64_t dtb_addr = 0x9000000000300000;
    
    uint64_t cmdline_addr = 0;
    uint64_t cmdline_size = 0;
    if (cmdline && strlen(cmdline) > 0) {
        cmdline_addr = dtb_addr + 0x100000;
        cmdline_size = strlen(cmdline) + 1;
        // Use strncpy for buffer safety; assumes cmdline_addr points to a buffer of at least cmdline_size
        strncpy((char*)cmdline_addr, cmdline, cmdline_size - 1);
        ((char*)cmdline_addr)[cmdline_size - 1] = '\0';
    }

    if (kernel_data && kernel_size > 0) {
        memcpy((void*)kernel_load_addr, kernel_data, kernel_size);
    }
    
    struct loongarch64_boot_params* params = (struct loongarch64_boot_params*)0x9000000000100000;
    memset(params, 0, sizeof(struct loongarch64_boot_params));
    
    params->dtb_addr = dtb_addr;
    params->cmdline_addr = cmdline_addr;
    params->cmdline_size = cmdline_size;
    params->kernel_addr = kernel_load_addr;
    params->kernel_size = kernel_size;
    params->mem_start = 0x9000000000000000;
    params->mem_size = 0x800000000;
    params->acpi_rsdp = 0;
    params->efi_systab = 0;
    
    void (*kernel_entry)(uint64_t, uint64_t, uint64_t) = (void*)kernel_load_addr;
    kernel_entry(0, dtb_addr, (uint64_t)params);
    
    return 0;
}

int loongarch64_verify_kernel(const char* kernel_path) {
    uint8_t* kernel_data = NULL;
    uint32_t kernel_size = 0;
    
    if (load_file(kernel_path, &kernel_data, &kernel_size) != 0) {
        return -1;
    }
    
    struct loongarch64_linux_header* header = (struct loongarch64_linux_header*)kernel_data;
    
    if (header->magic == 0x4c4f4f4e) {
        return 0;
    }
    
    return -1;
} 