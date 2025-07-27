#include <stdint.h>
#include "compat.h"
#include <string.h>
#include "multiboot2.h"

extern void* allocate_memory(uint32_t size);
extern int load_file(const char* path, uint8_t** data, uint32_t* size);

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

int multiboot2_load_kernel(const char* kernel_path, const char* cmdline) {
    uint8_t* kernel_data = NULL;
    uint32_t kernel_size = 0;
    
    // Load kernel file
    if (load_file(kernel_path, &kernel_data, &kernel_size) != 0) {
        return -1;
    }
    
    // Parse Multiboot 2 header
    uint32_t* header = (uint32_t*)kernel_data;
    
    // Verify Multiboot 2 magic
    if (header[0] != MULTIBOOT2_HEADER_MAGIC) {
        return -1;
    }
    
    // Find header address and entry point
    uint32_t header_addr = 0;
    uint32_t load_addr = 0;
    uint32_t load_end_addr = 0;
    uint32_t bss_end_addr = 0;
    uint32_t entry_addr = 0;
    
    // Parse header tags
    uint32_t offset = 8;
    while (offset < kernel_size) {
        struct multiboot2_tag* tag = (struct multiboot2_tag*)(kernel_data + offset);
        
        if (tag->type == MULTIBOOT2_HEADER_TAG_END) {
            break;
        }
        
        if (tag->type == MULTIBOOT2_HEADER_TAG_INFORMATION_REQUEST) {
            // Handle information request
        } else if (tag->type == MULTIBOOT2_HEADER_TAG_ADDRESS) {
            struct multiboot2_header_tag_address* addr_tag = (struct multiboot2_header_tag_address*)tag;
            header_addr = addr_tag->header_addr;
            load_addr = addr_tag->load_addr;
            load_end_addr = addr_tag->load_end_addr;
            bss_end_addr = addr_tag->bss_end_addr;
        } else if (tag->type == MULTIBOOT2_HEADER_TAG_ENTRY_ADDRESS) {
            struct multiboot2_header_tag_entry_address* entry_tag = (struct multiboot2_header_tag_entry_address*)tag;
            entry_addr = entry_tag->entry_addr;
        }
        
        offset += tag->size;
        if (tag->size % 8 != 0) {
            offset += 8 - (tag->size % 8);
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
    
    // Setup Multiboot 2 info structure
    struct multiboot2_info* mb_info = (struct multiboot2_info*)0x90000;
    memset(mb_info, 0, sizeof(struct multiboot2_info));
    
    uint32_t info_offset = sizeof(struct multiboot2_info);
    
    // Add command line tag
    if (cmdline && strlen(cmdline) > 0) {
        struct multiboot2_tag_string* cmdline_tag = (struct multiboot2_tag_string*)((uint8_t*)mb_info + info_offset);
        cmdline_tag->type = MULTIBOOT2_TAG_TYPE_CMDLINE;
        cmdline_tag->size = sizeof(struct multiboot2_tag_string) + strlen(cmdline) + 1;
        strcpy(cmdline_tag->string, cmdline);
        
        info_offset += cmdline_tag->size;
        if (cmdline_tag->size % 8 != 0) {
            info_offset += 8 - (cmdline_tag->size % 8);
        }
    }
    
    // Add basic memory info tag
    struct multiboot2_tag_basic_meminfo* meminfo_tag = (struct multiboot2_tag_basic_meminfo*)((uint8_t*)mb_info + info_offset);
    meminfo_tag->type = MULTIBOOT2_TAG_TYPE_BASIC_MEMINFO;
    meminfo_tag->size = sizeof(struct multiboot2_tag_basic_meminfo);
    meminfo_tag->mem_lower = 640; // 640KB
    meminfo_tag->mem_upper = 0x7FE00; // Extended memory in KB
    
    info_offset += meminfo_tag->size;
    if (meminfo_tag->size % 8 != 0) {
        info_offset += 8 - (meminfo_tag->size % 8);
    }
    
    // Add boot device tag
    struct multiboot2_tag_bootdev* bootdev_tag = (struct multiboot2_tag_bootdev*)((uint8_t*)mb_info + info_offset);
    bootdev_tag->type = MULTIBOOT2_TAG_TYPE_BOOTDEV;
    bootdev_tag->size = sizeof(struct multiboot2_tag_bootdev);
    bootdev_tag->biosdev = 0x80; // First hard disk
    bootdev_tag->partition = 0;
    bootdev_tag->subpartition = 0;
    
    info_offset += bootdev_tag->size;
    if (bootdev_tag->size % 8 != 0) {
        info_offset += 8 - (bootdev_tag->size % 8);
    }
    
    // Add memory map tag
    struct multiboot2_tag_mmap* mmap_tag = (struct multiboot2_tag_mmap*)((uint8_t*)mb_info + info_offset);
    mmap_tag->type = MULTIBOOT2_TAG_TYPE_MMAP;
    mmap_tag->size = sizeof(struct multiboot2_tag_mmap) + 2 * sizeof(struct multiboot2_mmap_entry);
    mmap_tag->entry_size = sizeof(struct multiboot2_mmap_entry);
    mmap_tag->entry_version = 0;
    
    mmap_tag->entries[0].addr = 0;
    mmap_tag->entries[0].len = 0x100000; // 1MB
    mmap_tag->entries[0].type = MULTIBOOT2_MMAP_TYPE_AVAILABLE;
    mmap_tag->entries[0].zero = 0;
    
    mmap_tag->entries[1].addr = 0x100000;
    mmap_tag->entries[1].len = 0x7FF00000; // ~2GB
    mmap_tag->entries[1].type = MULTIBOOT2_MMAP_TYPE_AVAILABLE;
    mmap_tag->entries[1].zero = 0;
    
    info_offset += mmap_tag->size;
    if (mmap_tag->size % 8 != 0) {
        info_offset += 8 - (mmap_tag->size % 8);
    }
    
    // Add end tag
    struct multiboot2_tag* end_tag = (struct multiboot2_tag*)((uint8_t*)mb_info + info_offset);
    end_tag->type = MULTIBOOT2_TAG_TYPE_END;
    end_tag->size = sizeof(struct multiboot2_tag);
    
    info_offset += end_tag->size;
    
    // Set total size
    mb_info->total_size = info_offset;
    mb_info->reserved = 0;
    
    // Jump to kernel
    void (*kernel_entry)(uint32_t, struct multiboot2_info*) = (void*)entry_addr;
    kernel_entry(MULTIBOOT2_BOOTLOADER_MAGIC, mb_info);
    
    return 0;
}

int multiboot2_verify_kernel(const char* kernel_path) {
    uint8_t* kernel_data = NULL;
    uint32_t kernel_size = 0;
    
    if (load_file(kernel_path, &kernel_data, &kernel_size) != 0) {
        return -1;
    }
    
    uint32_t* header = (uint32_t*)kernel_data;
    
    // Check Multiboot 2 magic
    if (header[0] != MULTIBOOT2_HEADER_MAGIC) {
        return -1;
    }
    
    // Check architecture
    if (header[1] != MULTIBOOT2_ARCHITECTURE_I386) {
        return -1;
    }
    
    // Check header length
    if (header[2] < 8 || header[2] > kernel_size) {
        return -1;
    }
    
    // Check header checksum
    uint32_t checksum = 0;
    for (uint32_t i = 0; i < header[2] / 4; i++) {
        checksum += header[i];
    }
    if (checksum != 0) {
        return -1;
    }
    
    return 0;
} 

int boot_multiboot2_kernel(uint8_t* kernel_data, uint32_t kernel_size, const char* cmdline) {
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
    
    uint32_t kernel_entry = 0x100000;
    memcpy((void*)kernel_entry, kernel_data, kernel_size);
    
    void (*entry_point)(uint32_t, uint32_t) = (void*)kernel_entry;
    entry_point(0x36d76289, (uint32_t)info);
    
    return 0;
} 