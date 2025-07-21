#include <stdint.h>
#include <string.h>
#include "plugin.h"

#define MAX_PLUGINS 32
#define MAX_FUNCTIONS_PER_PLUGIN 16

struct plugin_registry {
    struct plugin plugins[MAX_PLUGINS];
    int plugin_count;
};

static struct plugin_registry registry;

void plugin_init(void) {
    registry.plugin_count = 0;
}

int plugin_register(const char* name, const char* version, plugin_init_func init, plugin_cleanup_func cleanup) {
    if (registry.plugin_count >= MAX_PLUGINS) return -1;
    
    strcpy(registry.plugins[registry.plugin_count].name, name);
    strcpy(registry.plugins[registry.plugin_count].version, version);
    registry.plugins[registry.plugin_count].init = init;
    registry.plugins[registry.plugin_count].cleanup = cleanup;
    registry.plugins[registry.plugin_count].loaded = 0;
    registry.plugin_count++;
    return 0;
}

int plugin_load(const char* name) {
    for (int i = 0; i < registry.plugin_count; i++) {
        if (strcmp(registry.plugins[i].name, name) == 0) {
            if (registry.plugins[i].init) {
                registry.plugins[i].init();
            }
            registry.plugins[i].loaded = 1;
            return 0;
        }
    }
    return -1;
}

int plugin_unload(const char* name) {
    for (int i = 0; i < registry.plugin_count; i++) {
        if (strcmp(registry.plugins[i].name, name) == 0) {
            if (registry.plugins[i].cleanup) {
                registry.plugins[i].cleanup();
            }
            registry.plugins[i].loaded = 0;
            return 0;
        }
    }
    return -1;
}

void plugin_list(void) {
    printf("Loaded plugins:\n");
    for (int i = 0; i < registry.plugin_count; i++) {
        printf("  %s v%s %s\n", 
               registry.plugins[i].name, 
               registry.plugins[i].version,
               registry.plugins[i].loaded ? "[LOADED]" : "[UNLOADED]");
    }
} 