#include "plugin_registry.h"
#include <string.h>
static struct plugin_entry { char name[64]; void* data; uint32_t size; } plugins[16];
static int plugin_count = 0;
int plugin_registry_register(const char* name, void* data, uint32_t size) {
    if (plugin_count < 16) {
        strncpy(plugins[plugin_count].name, name, 63);
        plugins[plugin_count].name[63] = 0;
        plugins[plugin_count].data = data;
        plugins[plugin_count].size = size;
        plugin_count++;
        return 0;
    }
    return -1;
}
void* plugin_registry_find(const char* name, uint32_t* size) {
    for (int i = 0; i < plugin_count; ++i) {
        if (strcmp(plugins[i].name, name) == 0) {
            if (size) *size = plugins[i].size;
            return plugins[i].data;
        }
    }
    return 0;
} 