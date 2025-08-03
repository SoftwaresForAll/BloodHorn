; PXE Real-Mode Assembly Interface
; Copyright 2025 Listedroot. all rights reserved
section .text
global pxe_init
global pxe_dhcp_discover
global pxe_tftp_read
global pxe_get_file_size
global pxe_cleanup
global pxe_udp_send
global pxe_udp_recv

pxe_init:
    push bp
    mov bp, sp

    mov ax, 0x5650
    int 0x1A
    jc .no_pxe

    mov [pxe_api_entry_seg], es
    mov [pxe_api_entry_off], bx

    mov es, [pxe_api_entry_seg]
    mov bx, [pxe_api_entry_off]
    mov ax, 0x0001
    call far [es:bx+4]

    mov ax, 0
    jmp .done

.no_pxe:
    mov ax, 1

.done:
    pop bp
    ret

pxe_udp_send:
    push bp
    mov bp, sp
    push es
    push bx
    push si
    push di
    push cx
    push dx

    mov si, [bp+4]
    mov dx, [bp+6]
    mov di, [bp+8]
    mov cx, [bp+10]

    mov bx, si
    mov eax, [bx]
    mov edi, udp_dest_ip_buf
    mov [edi], eax

    mov es, [pxe_api_entry_seg]
    mov bx, [pxe_api_entry_off]
    mov ax, 0x0005
    mov si, di
    mov di, udp_dest_ip_buf
    call far [es:bx+4]
    jc .fail
    mov ax, 0
    jmp .done

.fail:
    mov ax, 1

.done:
    pop dx
    pop cx
    pop di
    pop si
    pop bx
    pop es
    pop bp
    ret

section .data
pxe_api_entry_seg: dw 0
pxe_api_entry_off: dw 0
transaction_id:    dd 0x12345678
mac_address:       db 0x00, 0x11, 0x22, 0x33, 0x44, 0x55
tftp_mode:         db "octet", 0
tftp_size_option:  db "tsize", 0, "0", 0
tftp_tsize_str:    db "tsize"
vendor_class:      db "PXEClient"

section .bss
dhcp_packet:       resb 300
dhcp_request:      resb 300
dhcp_response:     resb 576
tftp_packet:       resb 512
tftp_size_packet:  resb 512
tftp_data:         resb 516
tftp_oack_data:    resb 516
tftp_ack:          resb 4
tftp_error:        resb 516
block_number:      resw 1
tftp_retries:      resw 1
tftp_timeout:      resw 1
dhcp_state:        resw 1
dhcp_retries:      resw 1
dhcp_timeout:      resw 1
dhcp_msg_type:     resb 1
response_timeout:  resw 1
file_size_result:  resd 1
client_ip:         resd 1
offered_ip:        resd 1
server_ip:         resd 1
subnet_mask:       resd 1
router_ip:         resd 1
dns_server:        resd 1
tftp_server_name:  resb 64
boot_file_name:    resb 128
time_offset:       resd 1
domain_name:       resb 64
broadcast_ip:      resd 1
ntp_server:        resd 1
udp_dest_ip_buf:   resb 4
udp_recv_ip:       resb 4
udp_recv_port:     resw 1

DHCP_STATE_INIT    equ 0
DHCP_STATE_DISCOVER equ 1
DHCP_STATE_REQUEST equ 2
DHCP_STATE_BOUND   equ 3