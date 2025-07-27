#include "net_utils.h"
#include "compat.h"
#include <stdint.h>
uint16_t net_checksum(const uint8_t* data, int len) {
    uint32_t sum = 0;
    for (int i = 0; i < len; i += 2) {
        sum += (data[i]<<8) | data[i+1];
        if (sum > 0xFFFF) sum -= 0xFFFF;
    }
    return ~sum;
}
void net_mac_copy(uint8_t* dst, const uint8_t* src) {
    for (int i = 0; i < 6; ++i) dst[i] = src[i];
}
void net_ip_copy(uint8_t* dst, const uint8_t* src) {
    for (int i = 0; i < 4; ++i) dst[i] = src[i];
} 