#include "config_json.h"
#include <stdint.h>
#include <string.h>

int config_json_parse(const char* json, struct config_json* out) {
    const char* p = json;
    while (*p) {
        if (*p == '"') {
            const char* key = ++p;
            while (*p && *p != '"') p++;
            int keylen = p - key;
            p++;
            while (*p && *p != ':') p++;
            p++;
            while (*p == ' ' || *p == '"') p++;
            const char* value = p;
            while (*p && *p != '"' && *p != ',' && *p != '}') p++;
            int vallen = p - value;
            if (keylen < 32 && vallen < 128) {
                memcpy(out->key, key, keylen); out->key[keylen] = 0;
                memcpy(out->value, value, vallen); out->value[vallen] = 0;
                return 0;
            }
        }
        p++;
    }
    return -1;
} 