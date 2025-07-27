#include <stdint.h>
#include "compat.h"
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>
#include "plugin.h"

#define MAX_PLUGINS 32

struct plugin_handle {
    void* handle;
    struct plugin info;
};

static struct plugin_handle plugins[MAX_PLUGINS];
static int plugin_count = 0;

void plugin_init(void) {
    plugin_count = 0;
}

int plugin_register(const char* path) {
    if (plugin_count >= MAX_PLUGINS) return -1;
    void* handle = dlopen(path, RTLD_NOW);
    if (!handle) return -1;
    struct plugin* info = (struct plugin*)dlsym(handle, "plugin_info");
    if (!info) { dlclose(handle); return -1; }
    plugins[plugin_count].handle = handle;
    memcpy(&plugins[plugin_count].info, info, sizeof(struct plugin));
    if (plugins[plugin_count].info.init) plugins[plugin_count].info.init();
    plugin_count++;
    return 0;
}

int plugin_load(const char* name) {
    for (int i = 0; i < plugin_count; i++) {
        if (strcmp(plugins[i].info.name, name) == 0) {
            if (plugins[i].info.init) plugins[i].info.init();
            return 0;
        }
    }
    return -1;
}

int plugin_unload(const char* name) {
    for (int i = 0; i < plugin_count; i++) {
        if (strcmp(plugins[i].info.name, name) == 0) {
            if (plugins[i].info.cleanup) plugins[i].info.cleanup();
            dlclose(plugins[i].handle);
            for (int j = i; j < plugin_count-1; ++j) plugins[j] = plugins[j+1];
            plugin_count--;
            return 0;
        }
    }
    return -1;
}

void plugin_list(void) {
    printf("Loaded plugins:\n");
    for (int i = 0; i < plugin_count; i++) {
        printf("  %s v%s %s\n", plugins[i].info.name, plugins[i].info.version, "[LOADED]");
    }
} 