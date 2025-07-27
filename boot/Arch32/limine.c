#include <stdint.h>
#include "compat.h"
#include <string.h>
#include "limine.h"

extern void* allocate_memory(uint32_t size);
extern int load_file(const char* path, uint8_t** data, uint32_t* size);

struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

struct limine_smbios_request smbios_request = {
    .id = LIMINE_SMBIOS_REQUEST,
    .revision = 0
};

struct limine_efi_system_table_request efi_system_table_request = {
    .id = LIMINE_EFI_SYSTEM_TABLE_REQUEST,
    .revision = 0
};

struct limine_boot_time_request boot_time_request = {
    .id = LIMINE_BOOT_TIME_REQUEST,
    .revision = 0
};

struct limine_kernel_file_request kernel_file_request = {
    .id = LIMINE_KERNEL_FILE_REQUEST,
    .revision = 0
};

struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0
};

struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 0
};

struct limine_5_level_paging_request level5_paging_request = {
    .id = LIMINE_5_LEVEL_PAGING_REQUEST,
    .revision = 0
};

struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

struct limine_stack_size_request stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 0
};

struct limine_base_virtual_address_request base_virtual_address_request = {
    .id = LIMINE_BASE_VIRTUAL_ADDRESS_REQUEST,
    .revision = 0
};

struct limine_kernel_address_virtual_request kernel_address_virtual_request = {
    .id = LIMINE_KERNEL_ADDRESS_VIRTUAL_REQUEST,
    .revision = 0
};

struct limine_dtb_request dtb_request = {
    .id = LIMINE_DTB_REQUEST,
    .revision = 0
};

struct limine_vmap_request vmap_request = {
    .id = LIMINE_VMAP_REQUEST,
    .revision = 0
};

struct limine_loader_info_request loader_info_request = {
    .id = LIMINE_LOADER_INFO_REQUEST,
    .revision = 0
};

struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

int limine_load_kernel(const char* kernel_path, const char* cmdline) {
    uint8_t* kernel_data = NULL;
    uint32_t kernel_size = 0;
    
    // Load kernel file
    if (load_file(kernel_path, &kernel_data, &kernel_size) != 0) {
        return -1;
    }
    
    // Parse ELF header
    struct elf64_header* elf_header = (struct elf64_header*)kernel_data;
    
    // Verify ELF magic
    if (memcmp(elf_header->e_ident, ELF_MAGIC, 4) != 0) {
        return -1;
    }
    
    // Verify it's a 64-bit ELF
    if (elf_header->e_ident[EI_CLASS] != ELFCLASS64) {
        return -1;
    }
    
    // Load kernel to preferred address
    uint64_t load_addr = 0x200000; // 2MB
    if (elf_header->e_entry >= 0xffffffff80000000) {
        load_addr = elf_header->e_entry;
    }
    
    // Load ELF segments
    struct elf64_phdr* phdr = (struct elf64_phdr*)(kernel_data + elf_header->e_phoff);
    for (int i = 0; i < elf_header->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            uint64_t vaddr = phdr[i].p_vaddr;
            if (vaddr >= 0xffffffff80000000) {
                vaddr -= 0xffffffff80000000;
                vaddr += load_addr;
            }
            
            memcpy((void*)vaddr, kernel_data + phdr[i].p_offset, phdr[i].p_filesz);
            
            // Zero out BSS
            if (phdr[i].p_memsz > phdr[i].p_filesz) {
                memset((void*)(vaddr + phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);
            }
        }
    }
    
    // Setup Limine requests
    struct limine_memmap_response* memmap_response = allocate_memory(sizeof(struct limine_memmap_response));
    memmap_response->entry_count = 1;
    memmap_response->entries = allocate_memory(sizeof(struct limine_memmap_entry));
    memmap_response->entries[0].base = 0;
    memmap_response->entries[0].length = 0x100000000; // 4GB
    memmap_response->entries[0].type = LIMINE_MEMMAP_USABLE;
    
    struct limine_kernel_address_response* kernel_address_response = allocate_memory(sizeof(struct limine_kernel_address_response));
    kernel_address_response->physical_base = load_addr;
    kernel_address_response->virtual_base = 0xffffffff80000000;
    
    struct limine_hhdm_response* hhdm_response = allocate_memory(sizeof(struct limine_hhdm_response));
    hhdm_response->offset = 0xffff800000000000;
    
    struct limine_framebuffer_response* framebuffer_response = allocate_memory(sizeof(struct limine_framebuffer_response));
    framebuffer_response->framebuffer_count = 1;
    framebuffer_response->framebuffers = allocate_memory(sizeof(struct limine_framebuffer));
    framebuffer_response->framebuffers[0].address = 0xb8000;
    framebuffer_response->framebuffers[0].width = 80;
    framebuffer_response->framebuffers[0].height = 25;
    framebuffer_response->framebuffers[0].pitch = 160;
    framebuffer_response->framebuffers[0].bpp = 16;
    framebuffer_response->framebuffers[0].memory_model = 1;
    framebuffer_response->framebuffers[0].red_mask_size = 5;
    framebuffer_response->framebuffers[0].red_mask_shift = 11;
    framebuffer_response->framebuffers[0].green_mask_size = 6;
    framebuffer_response->framebuffers[0].green_mask_shift = 5;
    framebuffer_response->framebuffers[0].blue_mask_size = 5;
    framebuffer_response->framebuffers[0].blue_mask_shift = 0;
    
    // Jump to kernel
    void (*kernel_entry)(void) = (void*)elf_header->e_entry;
    kernel_entry();
    
    return 0;
}

int limine_verify_kernel(const char* kernel_path) {
    uint8_t* kernel_data = NULL;
    uint32_t kernel_size = 0;
    
    if (load_file(kernel_path, &kernel_data, &kernel_size) != 0) {
        return -1;
    }
    
    struct elf64_header* elf_header = (struct elf64_header*)kernel_data;
    
    // Check ELF magic
    if (memcmp(elf_header->e_ident, ELF_MAGIC, 4) != 0) {
        return -1;
    }
    
    // Check architecture
    if (elf_header->e_machine != EM_X86_64) {
        return -1;
    }
    
    return 0;
} 

int boot_limine_kernel(uint8_t* kernel_data, uint32_t kernel_size, const char* cmdline) {
    struct elf64_header* elf_header = (struct elf64_header*)kernel_data;
    
    if (elf_header->e_ident[0] != 0x7F || elf_header->e_ident[1] != 'E' || 
        elf_header->e_ident[2] != 'L' || elf_header->e_ident[3] != 'F') {
        return -1;
    }
    
    if (elf_header->e_type != 2) {
        return -1;
    }
    
    uint64_t entry_point = elf_header->e_entry;
    struct elf64_phdr* phdr = (struct elf64_phdr*)(kernel_data + elf_header->e_phoff);
    
    for (int i = 0; i < elf_header->e_phnum; i++) {
        if (phdr[i].p_type == 1) {
            memcpy((void*)phdr[i].p_vaddr, kernel_data + phdr[i].p_offset, phdr[i].p_filesz);
            if (phdr[i].p_memsz > phdr[i].p_filesz) {
                memset((void*)(phdr[i].p_vaddr + phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);
            }
        }
    }
    
    struct limine_kernel_file_response* kernel_response = (struct limine_kernel_file_response*)0x1000;
    kernel_response->revision = 0;
    kernel_response->kernel_file = (struct limine_file*)0x1100;
    kernel_response->kernel_file->revision = 0;
    kernel_response->kernel_file->address = (void*)0x100000;
    kernel_response->kernel_file->size = kernel_size;
    kernel_response->kernel_file->path = (char*)0x1200;
    kernel_response->kernel_file->cmdline = (char*)0x1300;
    
    if (cmdline && strlen(cmdline) > 0) {
        strcpy(kernel_response->kernel_file->cmdline, cmdline);
    }
    
    void (*entry)(void) = (void*)entry_point;
    entry();
    
    return 0;
} 