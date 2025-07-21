#include "tftp.h"
#include <stdint.h>
#include <string.h>
int tftp_build_rrq(const char* filename, uint8_t* buf) {
    buf[0] = 0; buf[1] = 1;
    int len = strlen(filename);
    memcpy(buf+2, filename, len);
    buf[2+len] = 0;
    memcpy(buf+3+len, "octet", 5);
    buf[8+len] = 0;
    return 9+len;
}
int tftp_parse_data(const uint8_t* buf, uint16_t* block, uint8_t* data, int* datalen) {
    if (buf[1] != 3) return -1;
    *block = (buf[2]<<8) | buf[3];
    *datalen = 512;
    memcpy(data, buf+4, 512);
    return 0;
} 