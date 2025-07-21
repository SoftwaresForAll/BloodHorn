#include "arp.h"
#include <stdint.h>
#include <string.h>
int arp_build_request(uint8_t* buf, const uint8_t* sender_mac, const uint8_t* sender_ip, const uint8_t* target_ip) {
    memset(buf, 0, 42);
    buf[0] = 0; buf[1] = 1; buf[2] = 8; buf[3] = 0; buf[4] = 6; buf[5] = 4; buf[6] = 0; buf[7] = 1;
    memcpy(buf+8, sender_mac, 6); memcpy(buf+14, sender_ip, 4);
    memset(buf+18, 0, 6); memcpy(buf+24, target_ip, 4);
    return 28;
} 