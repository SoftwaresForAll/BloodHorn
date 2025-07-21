#ifndef BLOODHORN_CONFIG_INI_H
#define BLOODHORN_CONFIG_INI_H

struct boot_menu_entry {
    char name[64];
    char path[128];
};

int parse_ini(const char* filename, struct boot_menu_entry* entries, int max_entries);

#endif // BLOODHORN_CONFIG_INI_H 