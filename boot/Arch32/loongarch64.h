#ifndef BLOODHORN_LOONGARCH64_H
#define BLOODHORN_LOONGARCH64_H
#include <stdint.h>

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

int loongarch64_load_kernel(const char* kernel_path, const char* initrd_path, const char* cmdline);
int loongarch64_boot_linux(uint8_t* kernel_data, uint64_t kernel_size, const char* initrd_path, const char* cmdline);
int loongarch64_boot_uefi(uint8_t* kernel_data, uint64_t kernel_size, const char* cmdline);
int loongarch64_verify_kernel(const char* kernel_path);

#endif 