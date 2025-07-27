#ifndef BLOODHORN_ISO9660_H
#define BLOODHORN_ISO9660_H
#include <stdint.h>
#include "compat.h"
int iso9660_read_file(uint32_t lba, const char* filename, uint8_t* buf, uint32_t max_size);
#endif 