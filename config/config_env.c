#include "config_env.h"
#include "compat.h"
#include <string.h>

int config_env_get(const char* key, char* value, int maxlen) {
    extern char** environ;
    for (char** e = environ; *e; ++e) {
        const char* eq = strchr(*e, '=');
        if (eq && strncmp(*e, key, eq - *e) == 0) {
            int len = strlen(eq + 1);
            if (len > maxlen - 1) len = maxlen - 1;
            memcpy(value, eq + 1, len); value[len] = 0;
            return 0;
        }
    }
    return -1;
} 