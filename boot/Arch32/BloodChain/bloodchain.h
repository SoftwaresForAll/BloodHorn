#ifndef BLOODCHAIN_H
#define BLOODCHAIN_H

#include <stdint.h>

// BloodChain Boot Protocol (BCBP) Specification
// Version: 1.0
// Magic: 0x424C4348 ("BLCH" in ASCII)

// Module Types
#define BCBP_MODTYPE_KERNEL     0x01  // OS Kernel
#define BCBP_MODTYPE_INITRD     0x02  // Initial RAM disk
#define BCBP_MODTYPE_ACPI       0x03  // ACPI tables
#define BCBP_MODTYPE_SMBIOS     0x04  // SMBIOS tables
#define BCBP_MODTYPE_DEVICETREE 0x05  // Device tree blob
#define BCBP_MODTYPE_EFI        0x06  // EFI runtime services
#define BCBP_MODTYPE_CONFIG     0x07  // Configuration file
#define BCBP_MODTYPE_DRIVER     0x08  // Hardware driver

// Boot Information Structure
struct bcbp_header {
    uint32_t magic;          // 0x424C4348 ("BLCH")
    uint32_t version;        // Protocol version (1.0 = 0x00010000)
    uint64_t entry_point;    // 64-bit entry point address
    uint64_t flags;          // Boot flags
    uint64_t boot_device;    // Boot device identifier
    uint64_t acpi_rsdp;      // ACPI RSDP address (0 if not available)
    uint64_t smbios;         // SMBIOS entry point (0 if not available)
    uint64_t framebuffer;    // Framebuffer information (0 if not available)
    uint64_t module_count;   // Number of loaded modules
    uint64_t modules;        // Pointer to module list
    uint8_t secure_boot;     // Secure boot status (0=disabled, 1=enabled)
    uint8_t tpm_available;   // TPM available (0=no, 1=yes)
    uint8_t uefi_64bit;      // 64-bit UEFI (0=no, 1=yes)
    uint8_t reserved[5];     // Reserved for future use
    uint8_t signature[64];   // Cryptographic signature (optional)
} __attribute__((packed));

// Module Information Structure
struct bcbp_module {
    uint64_t start;          // Module start address
    uint64_t size;           // Module size in bytes
    uint64_t cmdline;        // Command line string (0 if none)
    uint64_t name;           // Module name string
    uint8_t type;            // Module type (see BCBP_MODTYPE_*)
    uint8_t reserved[7];     // Reserved for future use
} __attribute__((packed));

// Bootloader Interface (for bootloader implementation)
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the BCBP header structure
 * 
 * @param hdr          Pointer to the header structure to initialize
 * @param entry_point  Kernel entry point address
 * @param boot_device  Boot device identifier
 */
void bcbp_init(struct bcbp_header *hdr, uint64_t entry_point, uint64_t boot_device);

/**
 * Add a module to the BCBP structure
 * 
 * @param hdr      Pointer to the BCBP header
 * @param start    Physical start address of the module
 * @param size     Size of the module in bytes
 * @param name     Name of the module (will be copied)
 * @param type     Module type (BCBP_MODTYPE_*)
 * @param cmdline  Command line string for the module (optional, can be NULL)
 */
void bcbp_add_module(struct bcbp_header *hdr, uint64_t start, uint64_t size,
                    const char *name, uint8_t type, const char *cmdline);

/**
 * Find a module by name
 * 
 * @param hdr   Pointer to the BCBP header
 * @param name  Name of the module to find
 * @return      Pointer to the module, or NULL if not found
 */
struct bcbp_module *bcbp_find_module(struct bcbp_header *hdr, const char *name);

/**
 * Validate the BCBP structure
 * 
 * @param hdr  Pointer to the BCBP header
 * @return     0 if valid, negative error code otherwise
 */
int bcbp_validate(const struct bcbp_header *hdr);

/**
 * Set up ACPI RSDP pointer
 * 
 * @param hdr    Pointer to the BCBP header
 * @param rsdp   Physical address of the ACPI RSDP
 */
void bcbp_set_acpi_rsdp(struct bcbp_header *hdr, uint64_t rsdp);

/**
 * Set up SMBIOS entry point
 * 
 * @param hdr      Pointer to the BCBP header
 * @param smbios   Physical address of the SMBIOS entry point
 */
void bcbp_set_smbios(struct bcbp_header *hdr, uint64_t smbios);

/**
 * Set up framebuffer information
 * 
 * @param hdr          Pointer to the BCBP header
 * @param framebuffer  Physical address of the framebuffer
 */
void bcbp_set_framebuffer(struct bcbp_header *hdr, uint64_t framebuffer);

#ifdef __cplusplus
}
#endif

// Kernel Interface (for kernel implementation)
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the boot information structure
 * 
 * @return  Pointer to the BCBP header (passed in RDI by bootloader)
 */
static inline struct bcbp_header *bcbp_get_boot_info(void) {
    struct bcbp_header *hdr;
    asm volatile ("mov %0, rdi" : "=r" (hdr));
    return hdr;
}

/**
 * Get module by index
 * 
 * @param hdr  Pointer to the BCBP header
 * @param idx  Module index (0-based)
 * @return     Pointer to the module, or NULL if out of bounds
 */
static inline struct bcbp_module *bcbp_get_module(struct bcbp_header *hdr, uint64_t idx) {
    if (idx >= hdr->module_count) return NULL;
    return ((struct bcbp_module *)hdr->modules) + idx;
}

#ifdef __cplusplus
}
#endif

// Constants for bootloader use
#define BCBP_MAGIC     0x424C4348  // "BLCH"
#define BCBP_VERSION   0x00010000  // 1.0
#define BCBP_HEADER_SIZE  sizeof(struct bcbp_header)
#define BCBP_MODULE_SIZE  sizeof(struct bcbp_module)

#endif // BLOODCHAIN_H
