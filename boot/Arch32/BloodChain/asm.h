#ifndef BLOODCHAIN_ASM_H
#define BLOODCHAIN_ASM_H

#ifndef __ASSEMBLER__
#error "This header is for assembly files only"
#endif

// Offsets for the bcbp_header structure
.equ BCBP_MAGIC_OFFSET, 0
.equ BCBP_VERSION_OFFSET, 4
.equ BCBP_ENTRY_POINT_OFFSET, 8
.equ BCBP_FLAGS_OFFSET, 16
.equ BCBP_BOOT_DEVICE_OFFSET, 24
.equ BCBP_ACPI_RSDP_OFFSET, 32
.equ BCBP_SMBIOS_OFFSET, 40
.equ BCBP_FRAMEBUFFER_OFFSET, 48
.equ BCBP_MODULE_COUNT_OFFSET, 56
.equ BCBP_MODULES_OFFSET, 64
.equ BCBP_SECURE_BOOT_OFFSET, 72
.equ BCBP_TPM_AVAILABLE_OFFSET, 73
.equ BCBP_UEFI_64BIT_OFFSET, 74

// Module type constants
.equ BCBP_MODTYPE_KERNEL, 0x01
.equ BCBP_MODTYPE_INITRD, 0x02
.equ BCBP_MODTYPE_ACPI, 0x03
.equ BCBP_MODTYPE_SMBIOS, 0x04
.equ BCBP_MODTYPE_DEVICETREE, 0x05
.equ BCBP_MODTYPE_EFI, 0x06
.equ BCBP_MODTYPE_CONFIG, 0x07
.equ BCBP_MODTYPE_DRIVER, 0x08

// Magic number for BCBP header
.equ BCBP_MAGIC, 0x424C4348  // 'BLCH'

// Size of the BCBP header structure
.equ BCBP_HEADER_SIZE, 152

// Size of the module structure
.equ BCBP_MODULE_SIZE, 40

// Macro to initialize a BCBP header in assembly
.macro bcbp_init header_ptr, entry_point
    // Clear the header
    mov edi, \header_ptr
    xor eax, eax
    mov ecx, BCBP_HEADER_SIZE / 4
    rep stosd
    
    // Set magic and version
    mov dword [\header_ptr + BCBP_MAGIC_OFFSET], BCBP_MAGIC
    mov dword [\header_ptr + BCBP_VERSION_OFFSET], 0x00010000
    
    // Set entry point
    mov eax, \entry_point
    mov [\header_ptr + BCBP_ENTRY_POINT_OFFSET], eax
    
    // Initialize module list pointer
    lea eax, [\header_ptr + BCBP_HEADER_SIZE]
    mov [\header_ptr + BCBP_MODULES_OFFSET], eax
    
    // Module count starts at 0
    mov dword [\header_ptr + BCBP_MODULE_COUNT_OFFSET], 0
.endm

// Macro to add a module in assembly
// Usage: bcbp_add_module header_ptr, start, size, name_ptr, type, cmdline_ptr
.macro bcbp_add_module header_ptr, start, size, name_ptr, type, cmdline_ptr=0
    pusha
    
    // Get header pointer
    mov edi, \header_ptr
    
    // Get module count and calculate module pointer
    mov ecx, [edi + BCBP_MODULE_COUNT_OFFSET]
    mov eax, ecx
    mov ebx, BCBP_MODULE_SIZE
    mul ebx
    add eax, [edi + BCBP_MODULES_OFFSET]
    mov edx, eax  // edx = module pointer
    
    // Store module information
    mov eax, \start
    mov [edx], eax
    mov eax, \size
    mov [edx + 8], eax
    
    // Store name pointer
    lea eax, [edx + BCBP_MODULE_SIZE]  // Store name after module structure
    mov [edx + 24], eax
    
    // Copy name string
    mov esi, \name_ptr
    mov edi, eax
    .Lcopy_name\@:
        lodsb
        stosb
        test al, al
        jnz .Lcopy_name\@
    
    // Store command line if provided
    mov eax, \cmdline_ptr
    test eax, eax
    jz .Lno_cmdline\@
    
    mov [edx + 16], edi  // Store cmdline pointer
    mov esi, eax
    .Lcopy_cmdline\@:
        lodsb
        stosb
        test al, al
        jnz .Lcopy_cmdline\@
    jmp .Ldone_cmdline\@
    
    .Lno_cmdline\@:
    mov dword [edx + 16], 0
    
    .Ldone_cmdline\@:
    // Store module type
    mov al, \type
    mov [edx + 32], al
    
    // Increment module count
    inc dword [edi + BCBP_MODULE_COUNT_OFFSET]
    
    popa
.endm

#endif // BLOODCHAIN_ASM_H
