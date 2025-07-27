#ifndef BLOODHORN_PXE_H
#define BLOODHORN_PXE_H
#include <stdint.h>
#include "compat.h"

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

int pxe_network_init(void);
int pxe_load_kernel(const char* kernel_path, uint8_t** kernel_data, uint32_t* kernel_size);
int pxe_load_initrd(const char* initrd_path, uint8_t** initrd_data, uint32_t* initrd_size);
int pxe_boot_kernel(const char* kernel_path, const char* initrd_path, const char* cmdline);
int pxe_cleanup_network(void);
struct pxe_network_info* pxe_get_network_info(void);

#endif 