#include "dhcp.h"
#include <stdint.h>
#include <string.h>
int dhcp_build_discover(uint8_t* buf, int xid) {
    memset(buf, 0, 300);
    buf[0] = 1; buf[1] = 1; buf[2] = 6; buf[3] = 0;
    *(uint32_t*)&buf[4] = xid;
    buf[236] = 99; buf[237] = 130; buf[238] = 83; buf[239] = 99;
    buf[240] = 53; buf[241] = 1; buf[242] = 1; buf[243] = 255;
    return 244;
}
int dhcp_parse_offer(const uint8_t* buf, uint32_t* offered_ip) {
    *offered_ip = *(uint32_t*)&buf[16];
    return 0;
} 