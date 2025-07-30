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
    mov al, 0x10        ; 16 sectors (8 KB)
    mov ch, 0x00        ; Cylinder 0
    mov cl, 0x03        ; Sector 3 (LBA 2)
    mov dh, 0x00        ; Head 0
    mov dl, [BOOT_DRIVE]; Boot drive
    mov bx, 0x0000      ; Buffer offset (0)
    mov ax, 0x9000
    mov es, ax
    int 0x13
    jc .chs_fail
    jmp .enter_pm

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
    jmp .enter_pm

.lba_fail:
    dec byte [retry_count]
    jz .disk_error
    mov ah, 0x00
    int 0x13
    jmp .lba_retry

.enter_pm:
    lgdt [gdt_desc]

    mov eax, cr0
    or eax, 1
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
    jmp 0x08:0x0000       ; jump to loaded kernel at 0x9000:0x0000

msg db 'BloodHorn Stage 2 Loader', 0
BOOT_DRIVE db 0
retry_count db 3

dap:
    db 0x10               ; DAP size
    db 0x00               ; Reserved
    dw 0x0010             ; Number of sectors (16)
    dw 0x0000             ; Offset in ES:BX (0x0000)
    dw 0x9000             ; Segment in ES:BX (0x9000)
    dq 0x0000000000000002 ; LBA 2

disk_err db 'Stage2 Disk Error!', 0

times 510-($-$$) db 0
    dw 0xAA55 