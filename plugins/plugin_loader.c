#include "plugin_loader.h"
#include "plugin_registry.h"
#include <stdint.h>
#include <string.h>

int plugin_loader_load(const char* path) {
    extern int load_file(const char* path, uint8_t** data, uint32_t* size);
    uint8_t* data = 0; uint32_t size = 0;
    if (load_file(path, &data, &size) != 0) return -1;
    return plugin_registry_register(path, data, size);
} 