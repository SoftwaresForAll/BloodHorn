#ifndef BLOODHORN_TFTP_H
#define BLOODHORN_TFTP_H
#include <stdint.h>
int tftp_build_rrq(const char* filename, uint8_t* buf);
int tftp_parse_data(const uint8_t* buf, uint16_t* block, uint8_t* data, int* datalen);
#endif 