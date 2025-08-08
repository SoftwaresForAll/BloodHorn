# BloodChain Boot Protocol Specification

## 1. Overview
The BloodChain Boot Protocol (BCBP) is a modern, secure, and extensible boot protocol designed specifically for the BloodHorn bootloader. It provides a standardized way to load and execute operating system kernels and boot modules with support for modern security features.

## 2. Protocol Version
- Current Version: 1.0
- Magic Number: 0x424C4348 ("BLCH" in ASCII)

## 3. Boot Information Structure

```c
struct bcbp_header {
    uint32_t magic;          // 0x424C4348 ("BLCH")
    uint32_t version;        // Protocol version (1.0 = 0x00010000)
    uint64_t entry_point;    // 64-bit entry point address
    uint64_t flags;          // Boot flags (see below)
    uint64_t boot_device;    // Boot device identifier
    uint64_t acpi_rsdp;      // ACPI RSDP address (0 if not available)
    uint64_t smbios;         // SMBIOS entry point (0 if not available)
    uint64_t framebuffer;    // Framebuffer information (0 if not available)
    uint64_t module_count;   // Number of loaded modules
    uint64_t modules;        // Pointer to module list
    uint8_t  secure_boot;    // Secure boot status (0=disabled, 1=enabled)
    uint8_t  tpm_available;  // TPM available (0=no, 1=yes)
    uint8_t  uefi_64bit;     // 64-bit UEFI (0=no, 1=yes)
    uint8_t  reserved[5];    // Reserved for future use
    uint8_t  signature[64];  // Cryptographic signature (optional)
} __attribute__((packed));

// Module information structure
struct bcbp_module {
    uint64_t start;          // Module start address
    uint64_t size;           // Module size in bytes
    uint64_t cmdline;        // Command line string (0 if none)
    uint64_t name;           // Module name string
    uint8_t  type;           // Module type (see below)
    uint8_t  reserved[7];    // Reserved for future use
} __attribute__((packed));

// Module Types
#define BCBP_MODTYPE_KERNEL   0x01  // OS Kernel
#define BCBP_MODTYPE_INITRD   0x02  // Initial RAM disk
#define BCBP_MODTYPE_ACPI     0x03  // ACPI tables
#define BCBP_MODTYPE_SMBIOS   0x04  // SMBIOS tables
#define BCBP_MODTYPE_DEVICETREE 0x05 // Device tree blob
#define BCBP_MODTYPE_EFI      0x06  // EFI runtime services
#define BCBP_MODTYPE_CONFIG   0x07  // Configuration file
#define BCBP_MODTYPE_DRIVER   0x08  // Hardware driver
```

## 4. Boot Process

1. **Bootloader Initialization**
   - Initialize hardware and memory management
   - Set up virtual memory (if applicable)
   - Initialize console and debug output
   - Load and verify kernel and modules
   - Set up boot information structure

2. **Kernel Handoff**
   - Disable interrupts
   - Set up CPU state as per x86_64 System V ABI
   - Pass control to kernel entry point with:
     - RDI: Pointer to bcbp_header structure
     - RSI: 0 (reserved for future use)
     - RDX: 0 (reserved for future use)
     - RBP: 0 (mark end of stack frames)

## 5. Security Features

### 5.1 Secure Boot
- Optional cryptographic verification of all loaded modules
- Support for TPM-based measurements
- Chain of trust from firmware to OS kernel

### 5.2 Memory Protection
- W^X (Write XOR Execute) policy for all loaded code
- ASLR (Address Space Layout Randomization)
- Stack protection

## 6. Example Bootloader Implementation

```c
void load_kernel() {
    struct bcbp_header *hdr = (struct bcbp_header*)BOOT_INFO_ADDR;
    
    // Initialize header
    hdr->magic = 0x424C4348;
    hdr->version = 0x00010000;
    hdr->entry_point = (uint64_t)kernel_entry;
    hdr->flags = 0;
    hdr->secure_boot = check_secure_boot();
    hdr->tpm_available = check_tpm();
    hdr->uefi_64bit = is_uefi_64bit();
    
    // Load modules
    struct bcbp_module *mods = (struct bcbp_module*)((char*)hdr + sizeof(*hdr));
    hdr->modules = (uint64_t)mods;
    
    // Load kernel
    mods[0].start = load_module("kernel", &mods[0].size);
    mods[0].type = BCBP_MODTYPE_KERNEL;
    mods[0].name = (uint64_t)"kernel";
    
    // Load initrd if available
    if (has_initrd()) {
        mods[1].start = load_module("initrd", &mods[1].size);
        mods[1].type = BCBP_MODTYPE_INITRD;
        mods[1].name = (uint64_t)"initrd";
        hdr->module_count = 2;
    } else {
        hdr->module_count = 1;
    }
    
    // Set up ACPI/SMBIOS pointers if available
    hdr->acpi_rsdp = get_acpi_rsdp();
    hdr->smbios = get_smbios_ptr();
    
    // Jump to kernel
    jump_to_kernel(hdr);
}
```

## 7. Example Kernel Implementation

```c
// Kernel entry point
void _start(struct bcbp_header *hdr) {
    // Verify magic number
    if (hdr->magic != 0x424C4348) {
        panic("Invalid boot protocol");
    }
    
    // Initialize console
    init_console();
    
    // Print boot information
    printf("BloodChain Boot Protocol v%u.%u\n", 
           (hdr->version >> 16) & 0xFFFF, hdr->version & 0xFFFF);
    
    // Process modules
    struct bcbp_module *mods = (struct bcbp_module*)hdr->modules;
    for (uint64_t i = 0; i < hdr->module_count; i++) {
        printf("Module %llu: %s at 0x%llx (%llu bytes)\n",
               i, (char*)mods[i].name, mods[i].start, mods[i].size);
    }
    
    // Continue kernel initialization
    kernel_main(hdr);
}
```

## 8. References
- UEFI Specification 2.9
- Multiboot2 Specification 1.0
- Linux Boot Protocol 2.10
- x86_64 System V ABI

## 9. Revision History
- 1.0 (2025-08-08): Initial specification
