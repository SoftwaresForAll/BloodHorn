#include "iso9660.h"
#include <stdint.h>
#include <string.h>
extern void read_sector(uint32_t lba, uint8_t* buf);
struct iso9660_dir_record { uint8_t len; uint8_t ext_attr_len; uint32_t extent; uint32_t size; uint8_t rest[20]; } __attribute__((packed));
int iso9660_read_file(uint32_t lba, const char* filename, uint8_t* buf, uint32_t max_size) {
    uint8_t pvd[2048];
    read_sector(lba+16, pvd);
    if (pvd[0] != 1 || memcmp(pvd+1, "CD001", 5) != 0) return -1;
    uint32_t root_extent = *(uint32_t*)(pvd+156);
    uint32_t root_size = *(uint32_t*)(pvd+164);
    uint8_t dir[2048];
    read_sector(lba+root_extent, dir);
    uint32_t off = 0;
    while (off < root_size) {
        struct iso9660_dir_record* rec = (struct iso9660_dir_record*)(dir+off);
        if (rec->len == 0) break;
        char name[32];
        int nlen = dir[off+32];
        memcpy(name, dir+off+33, nlen); name[nlen] = 0;
        if (strcmp(name, filename) == 0) {
            uint32_t f_extent = rec->extent;
            uint32_t f_size = rec->size;
            if (f_size > max_size) f_size = max_size;
            read_sector(lba+f_extent, buf);
            return 0;
        }
        off += rec->len;
    }
    return -1;
} 