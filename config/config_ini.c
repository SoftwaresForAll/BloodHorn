// BloodHorn INI Config Parser
// Real implementation for boot menu entries
#include <stdio.h>
#include <string.h>
#include "config_ini.h"

int parse_ini(const char* filename, struct boot_menu_entry* entries, int max_entries) {
    FILE* f = fopen(filename, "r");
    if (!f) return -1;
    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < max_entries) {
        if (line[0] == '[') continue; // skip section headers
        char* eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        strncpy(entries[count].name, line, sizeof(entries[count].name)-1);
        strncpy(entries[count].path, eq+1, sizeof(entries[count].path)-1);
        entries[count].name[sizeof(entries[count].name)-1] = 0;
        entries[count].path[sizeof(entries[count].path)-1] = 0;
        count++;
    }
    fclose(f);
    return count;
} 