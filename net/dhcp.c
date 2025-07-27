#include "dhcp.h"
#include "compat.h"
#include <stdint.h>
#include <string.h>
static uint8_t dhcp_options[312];
static uint32_t dhcp_lease_time;
static uint32_t dhcp_renew_time;
static uint32_t dhcp_rebind_time;
int dhcp_build_discover(uint8_t* buf, int xid) {
    memset(buf, 0, 300);
    buf[0] = 1; buf[1] = 1; buf[2] = 6; buf[3] = 0;
    *(uint32_t*)&buf[4] = xid;
    buf[236] = 99; buf[237] = 130; buf[238] = 83; buf[239] = 99;
    int opt = 240;
    buf[opt++] = 53; buf[opt++] = 1; buf[opt++] = 1;
    buf[opt++] = 55; buf[opt++] = 4; buf[opt++] = 1; buf[opt++] = 3; buf[opt++] = 6; buf[opt++] = 15;
    buf[opt++] = 255;
    return opt;
}
int dhcp_parse_offer(const uint8_t* buf, uint32_t* offered_ip) {
    *offered_ip = *(uint32_t*)&buf[16];
    memcpy(dhcp_options, buf+240, 312);
    for (int i = 0; i < 312;) {
        uint8_t code = dhcp_options[i++];
        if (code == 0xFF) break;
        uint8_t len = dhcp_options[i++];
        if (code == 51) dhcp_lease_time = *(uint32_t*)&dhcp_options[i];
        if (code == 58) dhcp_renew_time = *(uint32_t*)&dhcp_options[i];
        if (code == 59) dhcp_rebind_time = *(uint32_t*)&dhcp_options[i];
        i += len;
    }
    return 0;
}
int dhcp_renew(uint8_t* buf, int xid) {
    memset(buf, 0, 300);
    buf[0] = 1; buf[1] = 1; buf[2] = 6; buf[3] = 0;
    *(uint32_t*)&buf[4] = xid;
    buf[236] = 99; buf[237] = 130; buf[238] = 83; buf[239] = 99;
    int opt = 240;
    buf[opt++] = 53; buf[opt++] = 1; buf[opt++] = 5;
    buf[opt++] = 255;
    return opt;
}
int dhcp_release(uint8_t* buf, int xid) {
    memset(buf, 0, 300);
    buf[0] = 1; buf[1] = 1; buf[2] = 6; buf[3] = 0;
    *(uint32_t*)&buf[4] = xid;
    buf[236] = 99; buf[237] = 130; buf[238] = 83; buf[239] = 99;
    int opt = 240;
    buf[opt++] = 53; buf[opt++] = 1; buf[opt++] = 7;
    buf[opt++] = 255;
    return opt;
} 