#include "script_env.h"
#include "compat.h"
#include <string.h>
static char env[16][64];
static int env_count = 0;
int script_env_set(const char* key, const char* value) {
    if (env_count < 16) {
        snprintf(env[env_count], 64, "%s=%s", key, value);
        env_count++;
        return 0;
    }
    return -1;
}
const char* script_env_get(const char* key) {
    for (int i = 0; i < env_count; ++i) {
        char* eq = strchr(env[i], '=');
        if (eq && strncmp(env[i], key, eq - env[i]) == 0) return eq + 1;
    }
    return 0;
} 