#include "shell_net.h"
#include "compat.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "net/pxe.h"

// Minimal ICMP echo implementation using PXE stack (if supported)
// Returns 0 on success, -1 on failure
int shell_cmd_ping(const char* host, char* out, int maxlen) {
    // This assumes PXE stack provides a function for ICMP echo (not standard, but can be extended)
    extern int pxe_icmp_echo(const char* host, int* rtt_ms);
    int rtt = 0;
    int result = pxe_icmp_echo(host, &rtt);
    if (result == 0) {
        snprintf(out, maxlen, "Reply from %s: time=%dms\n", host, rtt);
        return 0;
    } else {
        snprintf(out, maxlen, "No reply from %s\n", host);
        return -1;
    }
}

// Print real network interface info from PXE stack
int shell_cmd_ifconfig(char* out, int maxlen) {
    struct pxe_network_info* info = pxe_get_network_info();
    if (!info) {
        snprintf(out, maxlen, "No network info available\n");
        return -1;
    }
    uint8_t* ip = (uint8_t*)&info->client_ip;
    uint8_t* mask = (uint8_t*)&info->subnet_mask;
    uint8_t* gw = (uint8_t*)&info->router_ip;
    uint8_t* dns = (uint8_t*)&info->dns_server;
    snprintf(out, maxlen,
        "eth0: %u.%u.%u.%u\n  netmask: %u.%u.%u.%u\n  gateway: %u.%u.%u.%u\n  dns: %u.%u.%u.%u\n  tftp: %s\n  bootfile: %s\n  domain: %s\n",
        ip[0], ip[1], ip[2], ip[3],
        mask[0], mask[1], mask[2], mask[3],
        gw[0], gw[1], gw[2], gw[3],
        dns[0], dns[1], dns[2], dns[3],
        info->tftp_server,
        info->boot_file,
        info->domain_name);
    return 0;
} 