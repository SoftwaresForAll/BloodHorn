#include "script_loader.h"
#include <stdint.h>
#include <string.h>
int script_loader_load(const char* path, char* out, int maxlen) {
    extern int load_file(const char* path, uint8_t** data, uint32_t* size);
    uint8_t* data = 0; uint32_t size = 0;
    if (load_file(path, &data, &size) != 0) return -1;
    if (size > maxlen - 1) size = maxlen - 1;
    memcpy(out, data, size); out[size] = 0;
    return 0;
} 