; BloodHorn BIOS Stage 2 Loader (Stage 2)
; Loads the main bootloader or kernel from disk, switches to protected mode
; and jumps to the entry point.

BITS 16
ORG 0x8000

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov si, msg
.print_char:
    lodsb
    or al, al
    jz .load_main
    mov ah, 0x0E
    int 0x10
    jmp .print_char

.load_main:
    mov ah, 0x02        ; BIOS read sectors
    mov al, 0x10        ; 16 sectors (8KB)
    mov ch, 0x00        ; Cylinder 0
    mov cl, 0x03        ; Sector 3 (LBA 2)
    mov dh, 0x00        ; Head 0
    mov dl, [BOOT_DRIVE]; Boot drive
    mov bx, 0x9000      ; Buffer
    int 0x13
    jc .disk_error

    ; Set up GDT for protected mode
    lgdt [gdt_desc]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x08:protected_mode

.disk_error:
    mov si, disk_err
    call print_str
    jmp $

print_str:
    lodsb
    or al, al
    jz .ret
    mov ah, 0x0E
    int 0x10
    jmp print_str
.ret:
    ret

; GDT for protected mode
align 8
gdt:
    dq 0x0000000000000000
    dq 0x00cf9a000000ffff
    dq 0x00cf92000000ffff

gdt_desc:
    dw 23
    dd gdt

protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9000
    ; Jump to loaded code (C entry or kernel)
    jmp 0x08:0x9000

msg db 'BloodHorn Stage 2 Loader', 0
BOOT_DRIVE db 0
disk_err db 'Stage2 Disk Error!', 0 