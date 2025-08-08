; BloodChain Boot Protocol - Stage 2 Loader
bits 16
org 0x7E00

%define BCBP_MAGIC 0x424C4348  ; 'BLCH'
%define BCBP_VERSION 0x00010000

struc bcbp_header
    .magic:         resd 1
    .version:       resd 1
    .entry_point:   resq 1
    .flags:         resq 1
    .boot_device:   resq 1
    .acpi_rsdp:     resq 1
    .smbios:        resq 1
    .framebuffer:   resq 1
    .module_count:  resq 1
    .modules:       resq 1
    .secure_boot:   resb 1
    .tpm_available: resb 1
    .uefi_64bit:    resb 1
    .reserved:      resb 5
    .signature:     resb 64
endstruc

struc bcbp_module
    .start:     resq 1
    .size:      resq 1
    .cmdline:   resq 1
    .name:      resq 1
    .type:      resb 1
    .reserved:  resb 7
endstruc

start:
    cli
    mov ax, 0x0000
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Enable A20 line
    call enable_a20
    
    ; Load kernel from disk
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov ah, 0x02
    mov al, 32      ; Read 32 sectors (16KB)
    mov ch, 0
    mov cl, 18      ; Start from sector 18 (after stage2)
    mov dh, 0
    mov dl, [0x7C00 + 0x24]  ; Boot drive from BPB
    int 0x13
    jc disk_error

    ; Set up protected mode
    lgdt [gdt_descriptor]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x08:protected_mode

enable_a20:
    cli
    call .wait_input
    mov al, 0xAD
    out 0x64, al
    call .wait_input
    mov al, 0xD0
    out 0x64, al
    call .wait_output
    in al, 0x60
    push eax
    call .wait_input
    mov al, 0xD1
    out 0x64, al
    call .wait_input
    pop eax
    or al, 2
    out 0x60, al
    call .wait_input
    mov al, 0xAE
    out 0x64, al
    call .wait_input
    sti
    ret

.wait_input:
    in al, 0x64
    test al, 2
    jnz .wait_input
    ret

.wait_output:
    in al, 0x64
    test al, 1
    jz .wait_output
    ret

bits 32
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Set up BCBP header at 0x10000
    mov edi, 0x10000
    mov ecx, bcbp_header_size
    xor eax, eax
    rep stosb

    mov dword [0x10000 + bcbp_header.magic], BCBP_MAGIC
    mov dword [0x10000 + bcbp_header.version], BCBP_VERSION
    mov dword [0x10000 + bcbp_header.entry_point], 0x100000  ; Kernel entry point
    mov qword [0x10000 + bcbp_header.modules], 0x10100       ; Modules start after header
    mov byte [0x10000 + bcbp_header.secure_boot], 0
    mov byte [0x10000 + bcbp_header.tpm_available], 0
    mov byte [0x10000 + bcbp_header.uefi_64bit], 0

    ; Set up kernel module
    mov edi, 0x10100
    mov dword [edi + bcbp_module.start], 0x100000  ; Kernel loaded at 1MB
    mov dword [edi + bcbp_module.size], 0x10000    ; 64KB kernel
    mov dword [edi + bcbp_module.cmdline], cmdline_kernel
    mov dword [edi + bcbp_module.name], str_kernel
    mov byte [edi + bcbp_module.type], 0x01        ; Kernel type

    ; Set module count to 1 (just the kernel for now)
    mov dword [0x10000 + bcbp_header.module_count], 1

    ; Jump to kernel
    mov eax, 0x10000
    jmp [eax + bcbp_header.entry_point]

disk_error:
    mov si, msg_disk_error
    call print_string_16
    jmp $

print_string_16:
    pusha
    mov ah, 0x0E
.print_loop:
    lodsb
    or al, al
    jz .done
    int 0x10
    jmp .print_loop
.done:
    popa
    ret

; Strings
msg_disk_error: db "Disk error loading stage2!", 0
str_kernel: db "kernel", 0
cmdline_kernel: db "root=/dev/sda1 ro", 0

; GDT
gdt_start:
    dq 0x0000000000000000  ; Null descriptor
    dq 0x00CF9A000000FFFF  ; Code segment (0-4GB)
    dq 0x00CF92000000FFFF  ; Data segment (0-4GB)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 8192-($-$$) db 0  ; Pad to 8KB
