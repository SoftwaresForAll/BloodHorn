#include "config_json.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static void parse_json_object(const char** p, char* section, struct config_json* out, int* count, int max_entries) {
    while (**p) {
        while (**p && **p != '"' && **p != '}') (*p)++;
        if (**p == '}') { (*p)++; break; }
        if (**p != '"') break;
        const char* key = ++(*p);
        while (**p && **p != '"') (*p)++;
        int keylen = *p - key;
        (*p)++;
        while (**p && **p != ':') (*p)++;
        (*p)++;
        while (**p == ' ' || **p == '\n') (*p)++;
        if (**p == '{') {
            (*p)++;
            char new_section[64];
            snprintf(new_section, sizeof(new_section), "%s%s%s", section, section[0] ? "." : "", key);
            parse_json_object(p, new_section, out, count, max_entries);
        } else {
            while (**p == ' ' || **p == '"') (*p)++;
            const char* value = *p;
            int in_str = 0;
            int vallen = 0;
            if (**p == '"') {
                in_str = 1; value = ++(*p);
                while (**p && **p != '"') (*p)++;
                vallen = *p - value;
                (*p)++;
            } else {
                while (**p && **p != ',' && **p != '}' && **p != '\n') { (*p)++; vallen++; }
            }
            if (keylen < 32 && vallen < 128 && *count < max_entries) {
                char fullkey[64];
                snprintf(fullkey, sizeof(fullkey), "%s%s%s", section, section[0] ? "." : "", key);
                strncpy(out[*count].key, fullkey, 31); out[*count].key[31] = 0;
                strncpy(out[*count].value, value, vallen); out[*count].value[vallen] = 0;
                (*count)++;
            }
            while (**p && **p != ',' && **p != '}') (*p)++;
            if (**p == ',') (*p)++;
        }
    }
}

int config_json_parse(const char* json, struct config_json* out, int max_entries) {
    int count = 0;
    const char* p = json;
    while (*p && *p != '{') p++;
    if (*p == '{') {
        p++;
        parse_json_object(&p, "", out, &count, max_entries);
    }
    return count;
} 