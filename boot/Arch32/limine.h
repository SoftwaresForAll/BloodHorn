#ifndef BLOODHORN_LIMINE_H
#define BLOODHORN_LIMINE_H
#include <stdint.h>
#include "compat.h"

#define LIMINE_MEMMAP_REQUEST 0x67cf3d9d378a806f
#define LIMINE_ENTRY_REQUEST 0x13a86c035aa1c6d5
#define LIMINE_FRAMEBUFFER_REQUEST 0x9d5827dcd881dd75
#define LIMINE_TERMINAL_REQUEST 0xc8ac59310c2b0844
#define LIMINE_5_LEVEL_PAGING_REQUEST 0x94469551da9b3192
#define LIMINE_SMP_REQUEST 0x95a67b819a1b857e
#define LIMINE_MEMORY_MAP_REQUEST 0x67cf3d9d378a806f
#define LIMINE_KERNEL_FILE_REQUEST 0xad97e90e83f1ed67
#define LIMINE_MODULE_REQUEST 0x3e7e279702be32af
#define LIMINE_RSDP_REQUEST 0xc5e77b6b397e7b43
#define LIMINE_SMBIOS_REQUEST 0x9a904e30eb8c97c9
#define LIMINE_EFI_SYSTEM_TABLE_REQUEST 0x5ceba5163eaaf6d6
#define LIMINE_BOOT_TIME_REQUEST 0x502746e184c088aa
#define LIMINE_KERNEL_ADDRESS_REQUEST 0x71ba76863cc55f63
#define LIMINE_HHDM_REQUEST 0x48dcf1cb8ad2b852
#define LIMINE_STACK_SIZE_REQUEST 0xebeef7ca23e25b4b
#define LIMINE_BASE_VIRTUAL_ADDRESS_REQUEST 0x0c1e4a5c8f8c3941
#define LIMINE_KERNEL_ADDRESS_VIRTUAL_REQUEST 0x3464ae692c2d8f36
#define LIMINE_DTB_REQUEST 0xb40ddb48d54c097a
#define LIMINE_VMAP_REQUEST 0xb0fb35bf0c847f0f
#define LIMINE_LOADER_INFO_REQUEST 0xf55038d8e2a1202f
#define LIMINE_PAGING_MODE_REQUEST 0xc5e77c404f100d86

#define LIMINE_MEMMAP_USABLE 0
#define LIMINE_MEMMAP_RESERVED 1
#define LIMINE_MEMMAP_ACPI_RECLAIMABLE 2
#define LIMINE_MEMMAP_ACPI_NVS 3
#define LIMINE_MEMMAP_BAD_MEMORY 4
#define LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define LIMINE_MEMMAP_KERNEL_AND_MODULES 6
#define LIMINE_MEMMAP_FRAMEBUFFER 7

#define ELF_MAGIC "\x7f\x45\x4c\x46"
#define ELFCLASS64 2
#define EM_X86_64 62
#define PT_LOAD 1

struct elf64_header {
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};

struct elf64_phdr {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
};

struct limine_memmap_entry {
    uint64_t base;
    uint64_t length;
    uint64_t type;
};

struct limine_memmap_response {
    uint64_t revision;
    uint64_t entry_count;
    struct limine_memmap_entry* entries;
};

struct limine_memmap_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_memmap_response* response;
};

struct limine_framebuffer {
    void* address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint16_t bpp;
    uint8_t memory_model;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
    uint8_t unused[7];
    uint64_t edid_size;
    void* edid;
};

struct limine_framebuffer_response {
    uint64_t revision;
    uint64_t framebuffer_count;
    struct limine_framebuffer** framebuffers;
};

struct limine_framebuffer_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_framebuffer_response* response;
};

struct limine_kernel_address_response {
    uint64_t revision;
    uint64_t physical_base;
    uint64_t virtual_base;
};

struct limine_kernel_address_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_kernel_address_response* response;
};

struct limine_hhdm_response {
    uint64_t revision;
    uint64_t offset;
};

struct limine_hhdm_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_hhdm_response* response;
};

struct limine_module_response {
    uint64_t revision;
    uint64_t module_count;
    struct limine_file** modules;
};

struct limine_module_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_module_response* response;
};

struct limine_file {
    uint64_t revision;
    void* address;
    uint64_t size;
    char* path;
    char* cmdline;
};

struct limine_rsdp_response {
    uint64_t revision;
    void* address;
};

struct limine_rsdp_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_rsdp_response* response;
};

struct limine_smbios_response {
    uint64_t revision;
    void* entry_32;
    void* entry_64;
};

struct limine_smbios_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_smbios_response* response;
};

struct limine_efi_system_table_response {
    uint64_t revision;
    void* address;
};

struct limine_efi_system_table_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_efi_system_table_response* response;
};

struct limine_boot_time_response {
    uint64_t revision;
    int64_t boot_time;
};

struct limine_boot_time_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_boot_time_response* response;
};

struct limine_kernel_file_response {
    uint64_t revision;
    struct limine_file* kernel_file;
};

struct limine_kernel_file_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_kernel_file_response* response;
};

struct limine_smp_info {
    uint32_t processor_id;
    uint32_t lapic_id;
    uint64_t reserved;
    uint64_t goto_address;
    uint64_t extra_argument;
};

struct limine_smp_response {
    uint64_t revision;
    uint32_t flags;
    uint32_t bsp_lapic_id;
    uint64_t cpu_count;
    struct limine_smp_info* cpus;
};

struct limine_smp_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_smp_response* response;
};

struct limine_paging_mode_response {
    uint64_t revision;
    uint64_t mode;
};

struct limine_paging_mode_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_paging_mode_response* response;
};

struct limine_5_level_paging_response {
    uint64_t revision;
    uint64_t mode;
};

struct limine_5_level_paging_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_5_level_paging_response* response;
};

struct limine_terminal_response {
    uint64_t revision;
    uint64_t write;
    uint64_t columns;
    uint64_t rows;
};

struct limine_terminal_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_terminal_response* response;
};

struct limine_stack_size_response {
    uint64_t revision;
    uint64_t stack_size;
};

struct limine_stack_size_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_stack_size_response* response;
};

struct limine_base_virtual_address_response {
    uint64_t revision;
    uint64_t virtual_base;
};

struct limine_base_virtual_address_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_base_virtual_address_response* response;
};

struct limine_kernel_address_virtual_response {
    uint64_t revision;
    uint64_t virtual_base;
};

struct limine_kernel_address_virtual_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_kernel_address_virtual_response* response;
};

struct limine_dtb_response {
    uint64_t revision;
    void* dtb_ptr;
};

struct limine_dtb_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_dtb_response* response;
};

struct limine_vmap_response {
    uint64_t revision;
    void* address;
};

struct limine_vmap_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_vmap_response* response;
};

struct limine_loader_info_response {
    uint64_t revision;
    char* name;
    char* version;
};

struct limine_loader_info_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_loader_info_response* response;
};

int limine_load_kernel(const char* kernel_path, const char* cmdline);
int limine_verify_kernel(const char* kernel_path);
int boot_limine_kernel(uint8_t* kernel_data, uint32_t kernel_size, const char* cmdline);

#endif // BLOODHORN_LIMINE_H 