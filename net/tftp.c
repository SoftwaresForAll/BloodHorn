#include "tftp.h"
#include "compat.h"
#include <stdint.h>
#include <string.h>
static int tftp_send_rrq(const char* filename, uint8_t* buf, int blksize) {
    buf[0] = 0; buf[1] = 1;
    int len = strlen(filename);
    memcpy(buf+2, filename, len); buf[2+len] = 0;
    memcpy(buf+3+len, "octet", 5); buf[8+len] = 0;
    int opt = 9+len;
    if (blksize > 512) {
        memcpy(buf+opt, "blksize", 7); buf[opt+7] = 0;
        char blkstr[8];
        int n = sprintf(blkstr, "%d", blksize);
        memcpy(buf+opt+8, blkstr, n); buf[opt+8+n] = 0;
        opt += 9+n;
    }
    buf[opt++] = 0;
    return opt;
}
int tftp_build_rrq(const char* filename, uint8_t* buf) {
    return tftp_send_rrq(filename, buf, 512);
}
int tftp_parse_data(const uint8_t* buf, uint16_t* block, uint8_t* data, int* datalen) {
    if (buf[1] != 3) return -1;
    *block = (buf[2]<<8) | buf[3];
    *datalen = (buf[4] == 0 && buf[5] == 0) ? 0 : 512;
    memcpy(data, buf+4, *datalen);
    return 0;
}
int tftp_parse_oack(const uint8_t* buf, int* blksize) {
    int i = 2;
    while (buf[i]) {
        if (strcmp((char*)buf+i, "blksize") == 0) {
            i += 8;
            *blksize = atoi((char*)buf+i);
            break;
        }
        while (buf[i++]);
    }
    return 0;
} 