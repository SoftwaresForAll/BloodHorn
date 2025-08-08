#include "bloodchain.h"
#include <string.h>

// Internal function to calculate required memory for modules and strings
static size_t calculate_required_memory(const struct bcbp_header *hdr) {
    if (!hdr || hdr->magic != BCBP_MAGIC) return 0;
    
    size_t total = sizeof(struct bcbp_header);
    total += hdr->module_count * sizeof(struct bcbp_module);
    
    // Add space for module names and command lines
    const struct bcbp_module *mod = (const struct bcbp_module *)hdr->modules;
    for (uint64_t i = 0; i < hdr->module_count; i++) {
        if (mod[i].name) {
            total += strlen((const char *)mod[i].name) + 1;
        }
        if (mod[i].cmdline) {
            total += strlen((const char *)mod[i].cmdline) + 1;
        }
    }
    
    return total;
}

void bcbp_init(struct bcbp_header *hdr, uint64_t entry_point, uint64_t boot_device) {
    if (!hdr) return;
    
    // Clear the entire header
    memset(hdr, 0, sizeof(struct bcbp_header));
    
    // Set magic and version
    hdr->magic = BCBP_MAGIC;
    hdr->version = BCBP_VERSION;
    
    // Set entry point and boot device
    hdr->entry_point = entry_point;
    hdr->boot_device = boot_device;
    
    // Initialize module list (starts right after the header)
    hdr->modules = (uint64_t)(hdr + 1);
    hdr->module_count = 0;
    
    // Initialize security features (to be set by bootloader)
    hdr->secure_boot = 0;
    hdr->tpm_available = 0;
    hdr->uefi_64bit = 0;
}

void bcbp_add_module(struct bcbp_header *hdr, uint64_t start, uint64_t size,
                    const char *name, uint8_t type, const char *cmdline) {
    if (!hdr || hdr->magic != BCBP_MAGIC || !name || size == 0) return;
    
    // Get module array
    struct bcbp_module *mods = (struct bcbp_module *)hdr->modules;
    
    // Calculate where the string data starts (after all module structures)
    char *str_ptr = (char *)(mods + hdr->module_count + 1);
    
    // Find the end of the current string data
    for (uint64_t i = 0; i < hdr->module_count; i++) {
        if (mods[i].name) {
            const char *name_str = (const char *)mods[i].name;
            size_t len = strlen(name_str) + 1;
            if ((char *)name_str + len > str_ptr) {
                str_ptr = (char *)name_str + len;
            }
        }
        if (mods[i].cmdline) {
            const char *cmd_str = (const char *)mods[i].cmdline;
            size_t len = strlen(cmd_str) + 1;
            if ((char *)cmd_str + len > str_ptr) {
                str_ptr = (char *)cmd_str + len;
            }
        }
    }
    
    // Add new module
    struct bcbp_module *mod = &mods[hdr->module_count++];
    
    // Set module fields
    mod->start = start;
    mod->size = size;
    mod->type = type;
    
    // Store name
    mod->name = (uint64_t)str_ptr;
    strcpy(str_ptr, name);
    str_ptr += strlen(name) + 1;
    
    // Store command line if provided
    if (cmdline && *cmdline) {
        mod->cmdline = (uint64_t)str_ptr;
        strcpy(str_ptr, cmdline);
        str_ptr += strlen(cmdline) + 1;
    } else {
        mod->cmdline = 0;
    }
}

struct bcbp_module *bcbp_find_module(struct bcbp_header *hdr, const char *name) {
    if (!hdr || hdr->magic != BCBP_MAGIC || !name) return NULL;
    
    struct bcbp_module *mod = (struct bcbp_module *)hdr->modules;
    
    for (uint64_t i = 0; i < hdr->module_count; i++) {
        if (mod[i].name && strcmp((const char *)mod[i].name, name) == 0) {
            return &mod[i];
        }
    }
    
    return NULL;
}

int bcbp_validate(const struct bcbp_header *hdr) {
    // Check pointer
    if (!hdr) return -1;
    
    // Check magic number
    if (hdr->magic != BCBP_MAGIC) return -2;
    
    // Check version compatibility (only major version matters for compatibility)
    if ((hdr->version >> 16) > (BCBP_VERSION >> 16)) {
        return -3; // Major version is higher than supported
    }
    
    // Check for reasonable module count
    if (hdr->module_count > 1024) return -4;
    
    // If we have modules, verify the modules pointer is valid
    if (hdr->module_count > 0) {
        // Modules must start after the header
        if (hdr->modules <= (uint64_t)hdr || 
            hdr->modules >= (uint64_t)hdr + calculate_required_memory(hdr)) {
            return -5; // Invalid modules pointer
        }
        
        // Verify each module
        const struct bcbp_module *mod = (const struct bcbp_module *)hdr->modules;
        for (uint64_t i = 0; i < hdr->module_count; i++) {
            // Check module type is valid
            if (mod[i].type < BCBP_MODTYPE_KERNEL || mod[i].type > BCBP_MODTYPE_DRIVER) {
                return -6; // Invalid module type
            }
            
            // Check name pointer is valid
            if (mod[i].name) {
                if (mod[i].name < (uint64_t)hdr || 
                    mod[i].name >= (uint64_t)hdr + calculate_required_memory(hdr)) {
                    return -7; // Invalid name pointer
                }
                if (strnlen((const char *)mod[i].name, 256) == 256) {
                    return -8; // Name too long or not null-terminated
                }
            }
            
            // Check command line pointer if present
            if (mod[i].cmdline) {
                if (mod[i].cmdline < (uint64_t)hdr || 
                    mod[i].cmdline >= (uint64_t)hdr + calculate_required_memory(hdr)) {
                    return -9; // Invalid command line pointer
                }
                if (strnlen((const char *)mod[i].cmdline, 4096) == 4096) {
                    return -10; // Command line too long or not null-terminated
                }
            }
        }
    }
    
    return 0; // Valid
}

void bcbp_set_acpi_rsdp(struct bcbp_header *hdr, uint64_t rsdp) {
    if (hdr && hdr->magic == BCBP_MAGIC) {
        hdr->acpi_rsdp = rsdp;
    }
}

void bcbp_set_smbios(struct bcbp_header *hdr, uint64_t smbios) {
    if (hdr && hdr->magic == BCBP_MAGIC) {
        hdr->smbios = smbios;
    }
}

void bcbp_set_framebuffer(struct bcbp_header *hdr, uint64_t framebuffer) {
    if (hdr && hdr->magic == BCBP_MAGIC) {
        hdr->framebuffer = framebuffer;
    }
}
