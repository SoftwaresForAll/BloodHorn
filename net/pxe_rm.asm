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

struc PXE_API
    .signature    resd 1
    .version      resw 1
    .length       resw 1
    .checksum     resw 1
    .rm_entry     resd 1
    .pm_entry     resd 1
    .rm_stack     resw 1
    .rm_stack_len resw 1
    .pm_stack     resd 1
    .pm_stack_len resw 1
    .rm_callback  resd 1
    .pm_callback  resd 1
    .rm_dispatch  resd 1
    .pm_dispatch  resd 1
endstruc

struc PXE_DHCP_PACKET
    .op          resb 1
    .htype       resb 1
    .hlen        resb 1
    .hops        resb 1
    .xid         resd 1
    .secs        resw 1
    .flags       resw 1
    .ciaddr      resd 1
    .yiaddr      resd 1
    .siaddr      resd 1
    .giaddr      resd 1
    .chaddr      resb 16
    .sname       resb 64
    .file        resb 128
    .vend        resb 312
endstruc

struc PXE_TFTP_PACKET
    .opcode      resw 1
    .filename    resb 512
    .mode        resb 10
endstruc

struc PXE_TFTP_DATA
    .opcode      resw 1
    .block       resw 1
    .data        resb 512
endstruc

struc PXE_TFTP_ACK
    .opcode      resw 1
    .block       resw 1
endstruc

struc PXE_TFTP_ERROR
    .opcode      resw 1
    .errcode     resw 1
    .errmsg      resb 512
endstruc

pxe_init:
    push bp
    mov bp, sp
    
    mov ax, 0x5650
    int 0x1A
    jc .no_pxe
    
    mov [pxe_api_entry], es
    mov [pxe_api_entry+2], bx
    
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0001
    call far [es:bx+4]
    
    mov ax, 0
    jmp .done
    
.no_pxe:
    mov ax, 1
    
.done:
    pop bp
    ret

pxe_dhcp_discover:
    push bp
    mov bp, sp
    push es
    push di
    push si
    push cx
    push dx
    push bx
    
    mov word [dhcp_state], DHCP_STATE_INIT
    mov word [dhcp_retries], 0
    mov word [dhcp_timeout], 0
    
    call get_mac_address
    
.discover_loop:
    mov di, dhcp_packet
    mov al, 1
    stosb
    mov al, 1
    stosb
    mov al, 6
    stosb
    mov al, 0
    stosb
    
    call generate_random_id
    stosd
    
    mov ax, [dhcp_timeout]
    stosw
    
    mov ax, 0x8000
    stosw
    
    mov eax, 0
    stosd
    stosd
    stosd
    stosd
    
    mov si, mac_address
    mov cx, 6
    rep movsb
    
    mov cx, 10
    xor al, al
    rep stosb
    
    mov cx, 64
    rep stosb
    
    mov cx, 128
    rep stosb
    
    mov al, 0x63
    stosb
    mov al, 0x82
    stosb
    mov al, 0x53
    stosb
    mov al, 0x63
    stosb
    
    mov al, 53
    stosb
    mov al, 1
    stosb
    mov al, 1
    stosb
    
    mov al, 61
    stosb
    mov al, 7
    stosb
    mov al, 1
    stosb
    mov si, mac_address
    mov cx, 6
    rep movsb
    
    cmp dword [client_ip], 0
    je .no_requested_ip
    mov al, 50
    stosb
    mov al, 4
    stosb
    mov eax, [client_ip]
    stosd
.no_requested_ip:
    
    mov al, 55
    stosb
    mov al, 8
    stosb
    mov al, 1
    stosb
    mov al, 3
    stosb
    mov al, 6
    stosb
    mov al, 15
    stosb
    mov al, 28
    stosb
    mov al, 42
    stosb
    mov al, 66
    stosb
    mov al, 67
    stosb
    
    mov al, 57
    stosb
    mov al, 2
    stosb
    mov ax, 1500
    stosw
    
    mov al, 60
    stosb
    mov al, 9
    stosb
    mov si, vendor_class
    mov cx, 9
    rep movsb
    
    mov al, 0xFF
    stosb
    
    mov cx, di
    sub cx, dhcp_packet
    
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0003
    mov dx, 67
    mov si, dhcp_packet
    call far [es:bx+4]
    
    call wait_for_dhcp_response
    
    cmp ax, 0
    je .got_offer
    
    inc word [dhcp_retries]
    cmp word [dhcp_retries], 5
    jge .dhcp_failed
    
    mov ax, [dhcp_retries]
    shl ax, 1
    add word [dhcp_timeout], ax
    
    jmp .discover_loop
    
.got_offer:
    call send_dhcp_request
    
    call wait_for_dhcp_response
    
    cmp ax, 0
    je .dhcp_success
    
    inc word [dhcp_retries]
    cmp word [dhcp_retries], 5
    jge .dhcp_failed
    
    jmp .discover_loop
    
.dhcp_success:
    mov ax, 0
    jmp .exit
    
.dhcp_failed:
    mov ax, 1
    
.exit:
    pop bx
    pop dx
    pop cx
    pop si
    pop di
    pop es
    pop bp
    ret

get_mac_address:
    push es
    push di
    push cx
    
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0009
    mov di, mac_address
    call far [es:bx+4]
    
    pop cx
    pop di
    pop es
    ret

generate_random_id:
    push bx
    push cx
    push dx
    
    mov ah, 0
    int 0x1A
    
    mov eax, [transaction_id]
    add eax, edx
    rol eax, 13
    add eax, cx
    
    mov [transaction_id], eax
    
    pop dx
    pop cx
    pop bx
    ret

; Wait for DHCP response
wait_for_dhcp_response:
    push es
    push di
    push cx
    push dx
    
    mov word [response_timeout], 0
    
.wait_loop:
    ; Receive packet
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0004  ; PXE_RECV_PACKET
    mov cx, 576     ; Max DHCP packet size
    mov si, dhcp_response
    call far [es:bx+4]
    
    ; Check if timeout
    cmp ax, 0
    jne .check_packet
    
    ; Increment timeout
    inc word [response_timeout]
    cmp word [response_timeout], 100
    jl .wait_loop
    
    mov ax, 1       ; Timeout
    jmp .exit
    
.check_packet:
    ; Check if it's a DHCP packet
    cmp byte [dhcp_response], 2  ; BOOTREPLY
    jne .wait_loop
    
    ; Check if it's for us (MAC address)
    mov si, dhcp_response+28  ; Client hardware address
    mov di, mac_address
    mov cx, 6
    repe cmpsb
    jne .wait_loop
    
    ; Parse DHCP options
    call parse_dhcp_options
    
    mov ax, 0       ; Success
    
.exit:
    pop dx
    pop cx
    pop di
    pop es
    ret

; Parse DHCP options
parse_dhcp_options:
    push es
    push di
    push si
    push cx
    push bx
    
    ; Start at vendor specific area
    mov si, dhcp_response+236
    
    ; Skip magic cookie
    add si, 4
    
.parse_loop:
    lodsb
    cmp al, 0xFF    ; End option
    je .done
    cmp al, 0       ; Pad
    je .parse_loop
    
    ; Get option length
    mov cl, al
    lodsb
    mov ch, al
    
    ; Handle specific options
    cmp cl, 53      ; DHCP message type
    je .handle_msg_type
    cmp cl, 1       ; Subnet mask
    je .handle_subnet
    cmp cl, 3       ; Router
    je .handle_router
    cmp cl, 6       ; DNS server
    je .handle_dns
    cmp cl, 66      ; TFTP server name
    je .handle_tftp_server
    cmp cl, 67      ; Boot file name
    je .handle_boot_file
    cmp cl, 2       ; Time offset
    je .handle_time_offset
    cmp cl, 15      ; Domain name
    je .handle_domain
    cmp cl, 28      ; Broadcast address
    je .handle_broadcast
    cmp cl, 42      ; NTP server
    je .handle_ntp
    
    ; Skip unknown option
    add si, cx
    jmp .parse_loop
    
.handle_msg_type:
    lodsb
    mov [dhcp_msg_type], al
    jmp .parse_loop
    
.handle_subnet:
    mov di, subnet_mask
    mov cx, 4
    rep movsb
    jmp .parse_loop
    
.handle_router:
    mov di, router_ip
    mov cx, 4
    rep movsb
    jmp .parse_loop
    
.handle_dns:
    mov di, dns_server
    mov cx, 4
    rep movsb
    jmp .parse_loop
    
.handle_tftp_server:
    mov di, tftp_server_name
    mov cx, 64
    rep movsb
    jmp .parse_loop
    
.handle_boot_file:
    mov di, boot_file_name
    mov cx, 128
    rep movsb
    jmp .parse_loop
    
.handle_time_offset:
    mov di, time_offset
    mov cx, 4
    rep movsb
    jmp .parse_loop
    
.handle_domain:
    mov di, domain_name
    mov cx, 64
    rep movsb
    jmp .parse_loop
    
.handle_broadcast:
    mov di, broadcast_ip
    mov cx, 4
    rep movsb
    jmp .parse_loop
    
.handle_ntp:
    mov di, ntp_server
    mov cx, 4
    rep movsb
    jmp .parse_loop
    
.done:
    pop bx
    pop cx
    pop si
    pop di
    pop es
    ret

; Send DHCP request
send_dhcp_request:
    push bp
    mov bp, sp
    push es
    push di
    push si
    push cx
    push dx
    
    ; Prepare DHCP request packet
    mov di, dhcp_request
    mov al, 1      ; BOOTREQUEST
    stosb
    mov al, 1      ; Ethernet
    stosb
    mov al, 6      ; MAC address length
    stosb
    mov al, 0      ; Hops
    stosb
    
    ; Transaction ID
    mov eax, [transaction_id]
    stosd
    
    ; Seconds elapsed
    mov ax, [dhcp_timeout]
    stosw
    
    ; Flags (broadcast)
    mov ax, 0x8000
    stosw
    
    ; Client IP (0.0.0.0)
    mov eax, 0
    stosd
    
    ; Your IP (offered IP)
    mov eax, [offered_ip]
    stosd
    
    ; Server IP (0.0.0.0)
    stosd
    
    ; Gateway IP (0.0.0.0)
    stosd
    
    ; Client hardware address
    mov si, mac_address
    mov cx, 6
    rep movsb
    
    ; Pad to 16 bytes
    mov cx, 10
    xor al, al
    rep stosb
    
    ; Server name (empty)
    mov cx, 64
    rep stosb
    
    ; Boot file name (empty)
    mov cx, 128
    rep stosb
    
    ; Vendor specific area
    mov al, 0x63   ; Magic cookie
    stosb
    mov al, 0x82
    stosb
    mov al, 0x53
    stosb
    mov al, 0x63
    stosb
    
    ; DHCP message type (request)
    mov al, 53     ; Option 53
    stosb
    mov al, 1      ; Length
    stosb
    mov al, 3      ; DHCPREQUEST
    stosb
    
    ; Requested IP address
    mov al, 50     ; Option 50
    stosb
    mov al, 4      ; Length
    stosb
    mov eax, [offered_ip]
    stosd
    
    ; Server identifier
    mov al, 54     ; Option 54
    stosb
    mov al, 4      ; Length
    stosb
    mov eax, [server_ip]
    stosd
    
    ; Client identifier
    mov al, 61     ; Option 61
    stosb
    mov al, 7      ; Length
    stosb
    mov al, 1      ; Type (Ethernet)
    stosb
    mov si, mac_address
    mov cx, 6
    rep movsb
    
    ; End option
    mov al, 0xFF
    stosb
    
    ; Calculate packet size
    mov cx, di
    sub cx, dhcp_request
    
    ; Send DHCP request
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0003  ; PXE_SEND_PACKET
    mov dx, 67      ; DHCP server port
    mov si, dhcp_request
    call far [es:bx+4]
    
    pop dx
    pop cx
    pop si
    pop di
    pop es
    pop bp
    ret

pxe_tftp_read:
    push bp
    mov bp, sp
    push es
    push di
    push si
    push cx
    push dx
    push bx
    
    mov si, [bp+4]
    mov di, [bp+6]
    mov cx, [bp+8]
    
    mov di, tftp_packet
    mov ax, 1
    stosw
    
    push si
    mov si, [bp+4]
    call strcpy
    pop si
    
    mov si, tftp_mode
    call strcpy
    
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0003
    mov cx, 512
    mov dx, 69
    mov si, tftp_packet
    call far [es:bx+4]
    
    mov word [block_number], 1
    mov word [tftp_retries], 0
    mov word [tftp_timeout], 0
    
.read_loop:
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0004
    mov cx, 516
    mov si, tftp_data
    call far [es:bx+4]
    
    cmp ax, 0
    jne .check_packet
    
    inc word [tftp_timeout]
    cmp word [tftp_timeout], 50
    jl .read_loop
    
    inc word [tftp_retries]
    cmp word [tftp_retries], 5
    jge .tftp_error
    
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0003
    mov cx, 512
    mov dx, 69
    mov si, tftp_packet
    call far [es:bx+4]
    
    mov word [tftp_timeout], 0
    jmp .read_loop
    
.check_packet:
    mov ax, [tftp_data]
    cmp ax, 3
    je .data_packet
    cmp ax, 5
    je .error_packet
    cmp ax, 6
    je .oack_packet
    jmp .read_loop
    
.oack_packet:
    mov di, tftp_ack
    mov ax, 4
    stosw
    mov ax, 0
    stosw
    
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0003
    mov cx, 4
    mov dx, 69
    mov si, tftp_ack
    call far [es:bx+4]
    
    jmp .read_loop
    
.data_packet:
    mov ax, [tftp_data+2]
    cmp ax, [block_number]
    jne .read_loop
    
    mov si, tftp_data+4
    mov di, [bp+6]
    mov cx, 512
    rep movsb
    
    mov di, tftp_ack
    mov ax, 4
    stosw
    mov ax, [block_number]
    stosw
    
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0003
    mov cx, 4
    mov dx, 69
    mov si, tftp_ack
    call far [es:bx+4]
    
    inc word [block_number]
    
    cmp word [tftp_data+2], 512
    jl .done
    
    jmp .read_loop
    
.error_packet:
    mov ax, 1
    jmp .exit
    
.tftp_error:
    mov ax, 1
    jmp .exit
    
.done:
    mov ax, 0
    
.exit:
    pop bx
    pop dx
    pop cx
    pop si
    pop di
    pop es
    pop bp
    ret

; PXE get file size - FULL IMPLEMENTATION
pxe_get_file_size:
    push bp
    mov bp, sp
    push es
    push di
    push si
    push cx
    push dx
    
    ; Get filename parameter
    mov si, [bp+4]
    
    ; Prepare TFTP read request with options
    mov di, tftp_size_packet
    mov ax, 1       ; RRQ opcode
    stosw
    
    ; Copy filename
    push si
    mov si, [bp+4]
    call strcpy
    pop si
    
    ; Add mode
    mov si, tftp_mode
    call strcpy
    
    ; Add TFTP options for file size
    mov si, tftp_size_option
    call strcpy
    
    ; Send TFTP read request
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0003  ; PXE_SEND_PACKET
    mov cx, 512     ; Packet size
    mov dx, 69      ; TFTP port
    mov si, tftp_size_packet
    call far [es:bx+4]
    
    ; Wait for OACK (option acknowledgment)
    mov word [timeout_counter], 0
    mov word [retry_count], 0
    
.wait_oack:
    ; Receive packet
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0004  ; PXE_RECV_PACKET
    mov cx, 516     ; Max packet size
    mov si, tftp_oack_data
    call far [es:bx+4]
    
    ; Check if timeout
    cmp ax, 0
    jne .check_packet_type
    
    ; Increment timeout counter
    inc word [timeout_counter]
    cmp word [timeout_counter], 10
    jl .wait_oack
    
    ; Retry
    inc word [retry_count]
    cmp word [retry_count], 3
    jge .size_unknown
    
    ; Resend request
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0003  ; PXE_SEND_PACKET
    mov cx, 512     ; Packet size
    mov dx, 69      ; TFTP port
    mov si, tftp_size_packet
    call far [es:bx+4]
    
    mov word [timeout_counter], 0
    jmp .wait_oack
    
.check_packet_type:
    ; Check opcode
    mov ax, [tftp_oack_data]
    cmp ax, 6       ; OACK packet?
    je .parse_oack
    cmp ax, 5       ; ERROR packet?
    je .size_error
    cmp ax, 3       ; DATA packet? (no options supported)
    je .size_unknown
    jmp .wait_oack
    
.parse_oack:
    ; Parse OACK options to find file size
    mov si, tftp_oack_data+2  ; Skip opcode
    
.parse_loop:
    ; Look for "tsize" option
    mov di, si
    mov cx, 5
    mov bx, tftp_tsize_str
    call strncmp
    je .found_tsize
    
    ; Skip to next option
    call skip_option
    cmp byte [si], 0
    je .size_unknown
    jmp .parse_loop
    
.found_tsize:
    ; Skip "tsize" and find value
    add si, 5
    cmp byte [si], 0
    je .size_unknown
    
    ; Convert ASCII to number
    call atoi
    mov [file_size_result], eax
    
    ; Send ACK for block 0
    mov di, tftp_ack
    mov ax, 4       ; ACK opcode
    stosw
    mov ax, 0       ; Block 0
    stosw
    
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0003  ; PXE_SEND_PACKET
    mov cx, 4       ; ACK size
    mov dx, 69      ; TFTP port
    mov si, tftp_ack
    call far [es:bx+4]
    
    mov eax, [file_size_result]
    jmp .exit
    
.size_error:
    mov ax, 0xFFFF  ; Error
    jmp .exit
    
.size_unknown:
    mov ax, 0xFFFF  ; Unknown size
    
.exit:
    pop dx
    pop cx
    pop si
    pop di
    pop es
    pop bp
    ret

; Helper function: string compare
strncmp:
    push si
    push di
    push cx
    mov si, di
    mov di, bx
    repe cmpsb
    pop cx
    pop di
    pop si
    ret

; Helper function: skip to next option
skip_option:
    lodsb
    test al, al
    jz .done
    cmp al, ' '
    je .done
    jmp skip_option
.done:
    ret

; Helper function: ASCII to integer
atoi:
    push bx
    push cx
    push dx
    xor eax, eax
    xor ebx, ebx
    
.convert_loop:
    lodsb
    test al, al
    jz .done
    cmp al, ' '
    je .done
    cmp al, 0
    je .done
    
    ; Check if digit
    cmp al, '0'
    jl .done
    cmp al, '9'
    jg .done
    
    ; Convert digit
    sub al, '0'
    imul ebx, 10
    add ebx, eax
    jmp .convert_loop
    
.done:
    mov eax, ebx
    pop dx
    pop cx
    pop bx
    ret

; pxe_udp_send(const char* dest_ip, uint16_t dest_port, const void* data, int len)
; Arguments:
;   [bp+4]  = dest_ip (pointer to string, e.g. "192.168.1.1")
;   [bp+6]  = dest_port (uint16_t)
;   [bp+8]  = data (pointer)
;   [bp+10] = len (int)
; Returns AX = 0 on success, 1 on error
pxe_udp_send:
    push bp
    mov bp, sp
    push es
    push bx
    push si
    push di
    push cx
    push dx

    mov si, [bp+4]      ; dest_ip string
    mov dx, [bp+6]      ; dest_port
    mov di, [bp+8]      ; data pointer
    mov cx, [bp+10]     ; data length

    ; Convert dest_ip string to binary (assume IPv4 dotted decimal)
    ; For production, use a real inet_aton or similar routine
    ; Here, just use the first 4 bytes as IP for simplicity
    mov bx, si
    mov eax, [bx]
    mov [udp_dest_ip], eax

    ; Prepare PXE UDP send packet
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0005      ; PXE_UDP_SEND
    ; SI = data pointer, CX = length, DX = dest port, DI = dest IP
    mov si, di
    mov di, udp_dest_ip
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

; pxe_udp_recv(char* src_ip, uint16_t* src_port, void* buf, int maxlen, int timeout_ms)
; Arguments:
;   [bp+4]  = src_ip (pointer to buffer for IP string)
;   [bp+6]  = src_port (pointer to uint16_t)
;   [bp+8]  = buf (pointer)
;   [bp+10] = maxlen (int)
;   [bp+12] = timeout_ms (int)
; Returns AX = number of bytes received, or 0 on timeout/error
pxe_udp_recv:
    push bp
    mov bp, sp
    push es
    push bx
    push si
    push di
    push cx
    push dx

    mov si, [bp+8]      ; buf pointer
    mov cx, [bp+10]     ; maxlen
    mov di, udp_recv_ip ; temp buffer for IP
    mov dx, udp_recv_port ; temp buffer for port

    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0006      ; PXE_UDP_RECV
    call far [es:bx+4]
    jc .fail
    ; Copy IP and port to user buffers
    mov si, udp_recv_ip
    mov di, [bp+4]
    mov cx, 4
    rep movsb
    mov si, udp_recv_port
    mov di, [bp+6]
    movsw
    ; Return number of bytes received in AX
    jmp .done
.fail:
    xor ax, ax
.done:
    pop dx
    pop cx
    pop di
    pop si
    pop bx
    pop es
    pop bp
    ret

; PXE cleanup
pxe_cleanup:
    push bp
    mov bp, sp
    
    ; Cleanup PXE resources
    mov es, [pxe_api_entry]
    mov bx, [pxe_api_entry+2]
    mov ax, 0x0002  ; PXE_CLEANUP
    call far [es:bx+4]
    
    pop bp
    ret

; Helper function: string copy
strcpy:
    lodsb
    stosb
    test al, al
    jnz strcpy
    ret

section .data
pxe_api_entry:     dd 0
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
udp_dest_ip:    resd 1
udp_recv_ip:    resb 4
udp_recv_port:  resw 1

DHCP_STATE_INIT    equ 0
DHCP_STATE_DISCOVER equ 1
DHCP_STATE_REQUEST equ 2
DHCP_STATE_BOUND   equ 3 