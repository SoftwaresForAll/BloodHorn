; BloodHorn BIOS/MBR Boot Sector (Stage 1)
; Loads stage 2 loader from LBA 1 (next sector)
; 512 bytes, bootable.

BITS 16
ORG 0x7C00

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [BOOT_DRIVE], dl

    mov si, msg
.print_char:
    lodsb
    or al, al
    jz .load_stage2
    mov ah, 0x0E
    int 0x10
    jmp .print_char

.load_stage2:
    mov ah, 0x02        ; BIOS read sectors
    mov al, 0x01        ; 1 sector
    mov ch, 0x00        ; Cylinder 0
    mov cl, 0x02        ; Sector 2 (LBA 1)
    mov dh, 0x00        ; Head 0
    mov dl, [BOOT_DRIVE]; Boot drive
    mov bx, 0x0000      ; Buffer offset
    mov ax, 0x8000
    mov es, ax
    int 0x13
    jc .disk_error

    jmp 0x0000:0x8000

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

msg db 'BloodHorn BIOS Bootloader', 0
BOOT_DRIVE db 0

disk_err db 'Disk Read Error!', 0

times 510-($-$$) db 0
    dw 0xAA55 