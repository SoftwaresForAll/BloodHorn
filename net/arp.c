#include "arp.h"
#include <stdint.h>
#include <string.h>
static uint8_t arp_cache_ip[4];
static uint8_t arp_cache_mac[6];
static int arp_cache_valid = 0;
int arp_build_request(uint8_t* buf, const uint8_t* sender_mac, const uint8_t* sender_ip, const uint8_t* target_ip) {
    memset(buf, 0, 42);
    buf[0] = 0; buf[1] = 1; buf[2] = 8; buf[3] = 0; buf[4] = 6; buf[5] = 4; buf[6] = 0; buf[7] = 1;
    memcpy(buf+8, sender_mac, 6); memcpy(buf+14, sender_ip, 4);
    memset(buf+18, 0, 6); memcpy(buf+24, target_ip, 4);
    return 28;
}
int arp_resolve(const uint8_t* sender_mac, const uint8_t* sender_ip, const uint8_t* target_ip, uint8_t* out_mac) {
    if (arp_cache_valid && memcmp(target_ip, arp_cache_ip, 4) == 0) {
        memcpy(out_mac, arp_cache_mac, 6);
        return 0;
    }
    uint8_t req[42];
    arp_build_request(req, sender_mac, sender_ip, target_ip);
    for (int retry = 0; retry < 3; ++retry) {
        send_ethernet(req, 42);
        for (int t = 0; t < 10000; ++t) {
            uint8_t resp[60];
            int n = recv_ethernet(resp, 60);
            if (n >= 42 && resp[12] == 0x08 && resp[13] == 0x06 && memcmp(resp+28, target_ip, 4) == 0) {
                memcpy(arp_cache_ip, target_ip, 4);
                memcpy(arp_cache_mac, resp+22, 6);
                arp_cache_valid = 1;
                memcpy(out_mac, resp+22, 6);
                return 0;
            }
        }
    }
    return -1;
} 