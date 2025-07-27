#ifndef BLOODHORN_MULTIBOOT2_H
#define BLOODHORN_MULTIBOOT2_H
#include <stdint.h>
#include "compat.h"

#define MULTIBOOT2_HEADER_MAGIC 0xE85250D6
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36D76289

#define MULTIBOOT2_ARCHITECTURE_I386 0
#define MULTIBOOT2_ARCHITECTURE_MIPS32 4

#define MULTIBOOT2_HEADER_TAG_END 0
#define MULTIBOOT2_HEADER_TAG_INFORMATION_REQUEST 1
#define MULTIBOOT2_HEADER_TAG_ADDRESS 2
#define MULTIBOOT2_HEADER_TAG_ENTRY_ADDRESS 3
#define MULTIBOOT2_HEADER_TAG_CONSOLE_FLAGS 4
#define MULTIBOOT2_HEADER_TAG_FRAMEBUFFER 5
#define MULTIBOOT2_HEADER_TAG_MODULE_ALIGN 6
#define MULTIBOOT2_HEADER_TAG_EFI_BS 7
#define MULTIBOOT2_HEADER_TAG_ENTRY_ADDRESS_EFI_32 8
#define MULTIBOOT2_HEADER_TAG_ENTRY_ADDRESS_EFI_64 9
#define MULTIBOOT2_HEADER_TAG_RELOCATABLE 10

#define MULTIBOOT2_TAG_TYPE_END 0
#define MULTIBOOT2_TAG_TYPE_CMDLINE 1
#define MULTIBOOT2_TAG_TYPE_BOOT_LOADER_NAME 2
#define MULTIBOOT2_TAG_TYPE_MODULE 3
#define MULTIBOOT2_TAG_TYPE_BASIC_MEMINFO 4
#define MULTIBOOT2_TAG_TYPE_BOOTDEV 5
#define MULTIBOOT2_TAG_TYPE_MMAP 6
#define MULTIBOOT2_TAG_TYPE_VBE 7
#define MULTIBOOT2_TAG_TYPE_FRAMEBUFFER 8
#define MULTIBOOT2_TAG_TYPE_ELF_SECTIONS 9
#define MULTIBOOT2_TAG_TYPE_APM 10
#define MULTIBOOT2_TAG_TYPE_EFI32 11
#define MULTIBOOT2_TAG_TYPE_EFI64 12
#define MULTIBOOT2_TAG_TYPE_SMBIOS 13
#define MULTIBOOT2_TAG_TYPE_ACPI_OLD 14
#define MULTIBOOT2_TAG_TYPE_ACPI_NEW 15
#define MULTIBOOT2_TAG_TYPE_NETWORK 16
#define MULTIBOOT2_TAG_TYPE_EFI_MMAP 17
#define MULTIBOOT2_TAG_TYPE_EFI_BS 18
#define MULTIBOOT2_TAG_TYPE_EFI32_IH 19
#define MULTIBOOT2_TAG_TYPE_EFI64_IH 20
#define MULTIBOOT2_TAG_TYPE_LOAD_BASE_ADDR 21

#define MULTIBOOT2_MMAP_TYPE_AVAILABLE 1
#define MULTIBOOT2_MMAP_TYPE_RESERVED 2
#define MULTIBOOT2_MMAP_TYPE_ACPI_RECLAIMABLE 3
#define MULTIBOOT2_MMAP_TYPE_ACPI_NVS 4
#define MULTIBOOT2_MMAP_TYPE_BAD_MEMORY 5
#define MULTIBOOT2_MMAP_TYPE_BOOTLOADER_RECLAIMABLE 0x1000
#define MULTIBOOT2_MMAP_TYPE_KERNEL_AND_MODULES 0x1001
#define MULTIBOOT2_MMAP_TYPE_FRAMEBUFFER 0x1002

struct multiboot2_info {
    uint32_t total_size;
    uint32_t reserved;
    uint8_t tags[];
};

struct multiboot2_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot2_tag_string {
    uint32_t type;
    uint32_t size;
    char string[];
};

struct multiboot2_tag_basic_meminfo {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
};

struct multiboot2_tag_bootdev {
    uint32_t type;
    uint32_t size;
    uint32_t biosdev;
    uint32_t partition;
    uint32_t subpartition;
};

struct multiboot2_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot2_mmap_entry entries[];
};

struct multiboot2_mmap_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
};

struct multiboot2_tag_vbe {
    uint32_t type;
    uint32_t size;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    struct multiboot2_vbe_info_block vbe_control_info;
    struct multiboot2_vbe_mode_info_block vbe_mode_info;
};

struct multiboot2_vbe_info_block {
    uint8_t vbe_signature[4];
    uint16_t vbe_version;
    uint32_t oem_string_ptr;
    uint32_t capabilities;
    uint32_t video_mode_ptr;
    uint16_t total_memory;
    uint16_t oem_software_rev;
    uint32_t oem_vendor_name_ptr;
    uint32_t oem_product_name_ptr;
    uint32_t oem_product_rev_ptr;
    uint8_t reserved[222];
    uint8_t oem_data[256];
};

struct multiboot2_vbe_mode_info_block {
    uint16_t mode_attributes;
    uint8_t win_a_attributes;
    uint8_t win_b_attributes;
    uint16_t win_granularity;
    uint16_t win_size;
    uint16_t win_a_segment;
    uint16_t win_b_segment;
    uint32_t win_func_ptr;
    uint16_t bytes_per_scan_line;
    uint16_t x_resolution;
    uint16_t y_resolution;
    uint8_t x_char_size;
    uint8_t y_char_size;
    uint8_t number_of_planes;
    uint8_t bits_per_pixel;
    uint8_t number_of_banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t number_of_image_pages;
    uint8_t reserved1;
    uint8_t red_mask_size;
    uint8_t red_field_position;
    uint8_t green_mask_size;
    uint8_t green_field_position;
    uint8_t blue_mask_size;
    uint8_t blue_field_position;
    uint8_t reserved_mask_size;
    uint8_t reserved_field_position;
    uint8_t direct_color_mode_info;
    uint32_t framebuffer;
    uint32_t off_screen_mem_offset;
    uint16_t off_screen_mem_size;
    uint8_t reserved2[206];
};

struct multiboot2_tag_framebuffer {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint8_t reserved;
    union {
        struct {
            uint32_t framebuffer_palette_num_colors;
            struct multiboot2_color_info framebuffer_palette[];
        };
        struct {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        };
    };
};

struct multiboot2_color_info {
    uint8_t red_value;
    uint8_t green_value;
    uint8_t blue_value;
};

struct multiboot2_tag_elf_sections {
    uint32_t type;
    uint32_t size;
    uint32_t num;
    uint32_t entsize;
    uint32_t shndx;
    char sections[];
};

struct multiboot2_tag_apm {
    uint32_t type;
    uint32_t size;
    uint16_t version;
    uint16_t cseg;
    uint32_t offset;
    uint16_t cseg_16;
    uint16_t dseg;
    uint16_t flags;
    uint16_t cseg_len;
    uint16_t cseg_16_len;
    uint16_t dseg_len;
};

struct multiboot2_tag_efi32 {
    uint32_t type;
    uint32_t size;
    uint32_t pointer;
};

struct multiboot2_tag_efi64 {
    uint32_t type;
    uint32_t size;
    uint64_t pointer;
};

struct multiboot2_tag_smbios {
    uint32_t type;
    uint32_t size;
    uint8_t major;
    uint8_t minor;
    uint8_t reserved[6];
    uint8_t tables[];
};

struct multiboot2_tag_old_acpi {
    uint32_t type;
    uint32_t size;
    uint8_t rsdp[];
};

struct multiboot2_tag_new_acpi {
    uint32_t type;
    uint32_t size;
    uint8_t rsdp[];
};

struct multiboot2_tag_network {
    uint32_t type;
    uint32_t size;
    uint8_t dhcpack[];
};

struct multiboot2_tag_efi_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t descr_size;
    uint32_t descr_vers;
    uint8_t efi_mmap[];
};

struct multiboot2_tag_efi32_ih {
    uint32_t type;
    uint32_t size;
    uint32_t pointer;
};

struct multiboot2_tag_efi64_ih {
    uint32_t type;
    uint32_t size;
    uint64_t pointer;
};

struct multiboot2_tag_load_base_addr {
    uint32_t type;
    uint32_t size;
    uint32_t load_base_addr;
};

struct multiboot2_header_tag_information_request {
    uint32_t type;
    uint32_t flags;
    uint32_t requests[];
};

struct multiboot2_header_tag_address {
    uint32_t type;
    uint32_t flags;
    uint32_t header_addr;
    uint32_t load_addr;
    uint32_t load_end_addr;
    uint32_t bss_end_addr;
};

struct multiboot2_header_tag_entry_address {
    uint32_t type;
    uint32_t flags;
    uint32_t entry_addr;
};

struct multiboot2_header_tag_console_flags {
    uint32_t type;
    uint32_t flags;
    uint32_t console_flags;
};

struct multiboot2_header_tag_framebuffer {
    uint32_t type;
    uint32_t flags;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
};

struct multiboot2_header_tag_module_align {
    uint32_t type;
    uint32_t flags;
};

struct multiboot2_header_tag_relocatable {
    uint32_t type;
    uint32_t flags;
    uint32_t min_addr;
    uint32_t max_addr;
    uint32_t align;
    uint32_t preference;
};

int multiboot2_load_kernel(const char* kernel_path, const char* cmdline);
int multiboot2_verify_kernel(const char* kernel_path);
int boot_multiboot2_kernel(uint8_t* kernel_data, uint32_t kernel_size, const char* cmdline);

#endif // BLOODHORN_MULTIBOOT2_H 