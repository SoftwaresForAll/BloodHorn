#include "script_api.h"
#include "lua.h"
#include <string.h>
static struct { char name[32]; void* fn; } cfuncs[16];
static int cfunc_count = 0;
int script_api_register(const char* name, void* fn) {
    if (cfunc_count < 16) {
        strncpy(cfuncs[cfunc_count].name, name, 31);
        cfuncs[cfunc_count].name[31] = 0;
        cfuncs[cfunc_count].fn = fn;
        cfunc_count++;
        return 0;
    }
    return -1;
}
void* script_api_lookup(const char* name) {
    for (int i = 0; i < cfunc_count; ++i) {
        if (strcmp(cfuncs[i].name, name) == 0) return cfuncs[i].fn;
    }
    return 0;
} 