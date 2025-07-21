#ifndef BLOODHORN_NET_UTILS_H
#define BLOODHORN_NET_UTILS_H
#include <stdint.h>
uint16_t net_checksum(const uint8_t* data, int len);
void net_mac_copy(uint8_t* dst, const uint8_t* src);
void net_ip_copy(uint8_t* dst, const uint8_t* src);
#endif 