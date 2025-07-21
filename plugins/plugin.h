#ifndef BLOODHORN_PLUGIN_H
#define BLOODHORN_PLUGIN_H

typedef void (*plugin_init_func)(void);
typedef void (*plugin_cleanup_func)(void);

struct plugin {
    char name[64];
    char version[16];
    plugin_init_func init;
    plugin_cleanup_func cleanup;
    int loaded;
};

void plugin_init(void);
int plugin_register(const char* name, const char* version, plugin_init_func init, plugin_cleanup_func cleanup);
int plugin_load(const char* name);
int plugin_unload(const char* name);
void plugin_list(void);

#endif // BLOODHORN_PLUGIN_H 