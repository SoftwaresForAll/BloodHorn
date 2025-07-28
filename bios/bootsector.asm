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
    jz .check_lba
    mov ah, 0x0E
    int 0x10
    jmp .print_char

.check_lba:
    mov ah, 0x41
    mov bx, 0x55AA
    int 0x13
    jc .use_chs
    cmp bx, 0xAA55
    jne .use_chs
    test cx, 1
    jz .use_chs
    jmp .load_lba

.use_chs:
    mov si, retry_count
.chs_retry:
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
    jc .chs_fail
    jmp 0x8000:0x0000

.chs_fail:
    dec byte [si]
    jz .disk_error
    mov ah, 0x00
    int 0x13
    jmp .chs_retry

.load_lba:
    mov si, retry_count
.lba_retry:
    mov si, dap
    mov ah, 0x42
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc .lba_fail
    jmp 0x8000:0x0000

.lba_fail:
    dec byte [retry_count]
    jz .disk_error
    mov ah, 0x00
    int 0x13
    jmp .lba_retry

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
retry_count db 3

dap:
    db 0x10               ; DAP size
    db 0x00               ; Reserved
    dw 0x0001             ; Number of sectors to read
    dw 0x0000             ; Offset in ES:BX (0x0000)
    dw 0x8000             ; Segment in ES:BX (0x8000)
    dq 0x0000000000000001 ; LBA 1

disk_err db 'Disk Read Error!', 0

times 510-($-$$) db 0
    dw 0xAA55 