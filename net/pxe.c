#include <stdint.h>
#include <string.h>
#include "pxe.h"
#include "boot/Arch32/linux.h"
#include "boot/Arch32/limine.h"
#include "boot/Arch32/multiboot1.h"
#include "boot/Arch32/multiboot2.h"
#include "boot/Arch32/chainload.h"
#include <time.h>
#include <arpa/inet.h>

extern int pxe_init(void);
extern int pxe_dhcp_discover(void);
extern int pxe_tftp_read(const char* filename, uint8_t* buffer, uint32_t size);
extern int pxe_get_file_size(const char* filename);
extern int pxe_cleanup(void);
extern int pxe_udp_send(const char* dest_ip, uint16_t dest_port, const void* data, int len);
extern int pxe_udp_recv(char* src_ip, uint16_t* src_port, void* buf, int maxlen, int timeout_ms);

struct pxe_network_info {
    uint32_t client_ip;
    uint32_t server_ip;
    uint32_t subnet_mask;
    uint32_t router_ip;
    uint32_t dns_server;
    char tftp_server[64];
    char boot_file[128];
    char domain_name[64];
    uint32_t broadcast_ip;
    uint32_t ntp_server;
    uint32_t time_offset;
};

static struct pxe_network_info network_info;
static int pxe_initialized = 0;

struct icmp_echo {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seq;
    uint8_t payload[32];
};

static uint16_t icmp_checksum(const void* data, int len) {
    uint32_t sum = 0;
    const uint16_t* ptr = (const uint16_t*)data;
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    if (len == 1) sum += *(const uint8_t*)ptr;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

int pxe_network_init(void) {
    if (pxe_initialized) {
        return 0;
    }
    
    int result = pxe_init();
    if (result != 0) {
        return -1;
    }
    
    result = pxe_dhcp_discover();
    if (result != 0) {
        pxe_cleanup();
        return -1;
    }
    
    pxe_initialized = 1;
    return 0;
}

int pxe_load_kernel(const char* kernel_path, uint8_t** kernel_data, uint32_t* kernel_size) {
    if (!pxe_initialized) {
        return -1;
    }
    
    *kernel_size = pxe_get_file_size(kernel_path);
    if (*kernel_size == 0xFFFF) {
        *kernel_size = 1024 * 1024; // Default 1MB if size unknown
    }
    
    *kernel_data = allocate_memory(*kernel_size);
    if (!*kernel_data) {
        return -1;
    }
    
    int result = pxe_tftp_read(kernel_path, *kernel_data, *kernel_size);
    if (result != 0) {
        return -1;
    }
    
    return 0;
}

int pxe_load_initrd(const char* initrd_path, uint8_t** initrd_data, uint32_t* initrd_size) {
    if (!pxe_initialized) {
        return -1;
    }
    
    *initrd_size = pxe_get_file_size(initrd_path);
    if (*initrd_size == 0xFFFF) {
        return -1;
    }
    
    *initrd_data = allocate_memory(*initrd_size);
    if (!*initrd_data) {
        return -1;
    }
    
    int result = pxe_tftp_read(initrd_path, *initrd_data, *initrd_size);
    if (result != 0) {
        return -1;
    }
    
    return 0;
}

int pxe_boot_kernel(const char* kernel_path, const char* initrd_path, const char* cmdline) {
    uint8_t* kernel_data = NULL;
    uint32_t kernel_size = 0;
    uint8_t* initrd_data = NULL;
    uint32_t initrd_size = 0;
    
    if (pxe_network_init() != 0) {
        return -1;
    }
    
    if (pxe_load_kernel(kernel_path, &kernel_data, &kernel_size) != 0) {
        return -1;
    }
    
    if (initrd_path && strlen(initrd_path) > 0) {
        if (pxe_load_initrd(initrd_path, &initrd_data, &initrd_size) != 0) {
            return -1;
        }
    }
    
    uint32_t* kernel_header = (uint32_t*)kernel_data;
    
    if (kernel_header[0] == 0x53726448) {
        return boot_linux_kernel(kernel_data, kernel_size, initrd_data, initrd_size, cmdline);
    }
    
    if (kernel_header[0] == 0x1BADB002) {
        return boot_multiboot1_kernel(kernel_data, kernel_size, cmdline);
    }
    
    if (kernel_header[0] == 0xE85250D6) {
        return boot_multiboot2_kernel(kernel_data, kernel_size, cmdline);
    }
    
    if (kernel_header[0] == 0x67cf3d9d) {
        return boot_limine_kernel(kernel_data, kernel_size, cmdline);
    }
    
    return -1;
}

int pxe_cleanup_network(void) {
    if (pxe_initialized) {
        pxe_cleanup();
        pxe_initialized = 0;
    }
    return 0;
}

struct pxe_network_info* pxe_get_network_info(void) {
    return &network_info;
} 

// ICMP echo (ping) using PXE stack
// Returns 0 on success, -1 on failure
int pxe_icmp_echo(const char* host, int* rtt_ms) {
    struct icmp_echo req = {0};
    req.type = 8; // Echo request
    req.code = 0;
    req.id = htons(0x1234);
    req.seq = htons(1);
    memset(req.payload, 0xAA, sizeof(req.payload));
    req.checksum = 0;
    req.checksum = icmp_checksum(&req, sizeof(req));

    uint16_t dest_port = 33434; // Arbitrary unused port
    uint32_t start = (uint32_t)clock();
    if (pxe_udp_send(host, dest_port, &req, sizeof(req)) < 0) return -1;

    char src_ip[16];
    uint16_t src_port;
    struct icmp_echo resp;
    int timeout = 1000; // 1s
    int n = pxe_udp_recv(src_ip, &src_port, &resp, sizeof(resp), timeout);
    if (n < (int)sizeof(resp)) return -1;
    if (resp.type != 0 || resp.id != req.id || resp.seq != req.seq) return -1;
    uint32_t end = (uint32_t)clock();
    *rtt_ms = (int)(((end - start) * 1000) / CLOCKS_PER_SEC);
    return 0;
} 