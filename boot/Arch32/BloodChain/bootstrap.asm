; BloodChain Boot Protocol - Bootstrap Loader
bits 16
org 0x7C00

; BIOS Parameter Block (FAT12/16/32 compatible)
jmp short start
nop

times 3-($-$$) db 0

; BPB (BIOS Parameter Block)
oem_id:            db "BLOODHOR"
bytes_per_sector:   dw 512
sectors_per_clust:  db 1
reserved_sects:     dw 1
num_fats:          db 2
root_entries:      dw 224
total_sectors:     dw 2880
media:             db 0xF0
sectors_per_fat:   dw 9
sectors_per_track: dw 18
num_heads:         dw 2
hidden_sects:      dd 0
total_sectors_lg:  dd 0

; Extended BPB
drive_num:         db 0
reserved:          db 0
boot_sig:          db 0x29
volume_id:         dd 0x12345678
volume_label:      db "BLOODHORN  "
file_system:       db "FAT12   "

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Save boot drive
    mov [drive_num], dl

    ; Load stage2 (next 16 sectors)
    mov ah, 0x02
    mov al, 16
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [drive_num]
    mov bx, 0x7E00
    int 0x13
    jc disk_error

    ; Jump to stage2
    jmp 0x0000:0x7E00

disk_error:
    mov si, msg_disk_error
    call print_string
    jmp $

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

msg_disk_error: db "Disk error!", 0

times 510-($-$$) db 0
dw 0xAA55
