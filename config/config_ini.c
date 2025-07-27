// BloodHorn INI Config Parser
// Real implementation for boot menu entries
#include <stdio.h>
#include "compat.h"
#include <string.h>
#include "config_ini.h"

int parse_ini(const char* filename, struct boot_menu_entry* entries, int max_entries) {
    FILE* f = fopen(filename, "r");
    if (!f) return -1;
    char line[256];
    char current_section[64] = "";
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < max_entries) {
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == ';' || *p == '#') continue;
        if (*p == '[') {
            char* end = strchr(p, ']');
            if (end) {
                int len = end - (p+1);
                if (len > 0 && len < 64) {
                    strncpy(current_section, p+1, len);
                    current_section[len] = 0;
                }
            }
            continue;
        }
        char* eq = strchr(p, '=');
        if (!eq) continue;
        *eq = 0;
        char* key = p;
        char* value = eq+1;
        while (*value == ' ' || *value == '\t') value++;
        char* nl = strchr(value, '\n');
        if (nl) *nl = 0;
        snprintf(entries[count].section, sizeof(entries[count].section), "%s", current_section);
        snprintf(entries[count].name, sizeof(entries[count].name), "%s", key);
        snprintf(entries[count].path, sizeof(entries[count].path), "%s", value);
        count++;
    }
    fclose(f);
    return count;
} 