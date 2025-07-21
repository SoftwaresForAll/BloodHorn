#include "iso9660.h"
#include <stdint.h>
#include <string.h>
extern void read_sector(uint32_t lba, uint8_t* buf);
struct iso9660_dir_record { uint8_t len; uint8_t ext_attr_len; uint32_t extent; uint32_t size; uint8_t rest[20]; } __attribute__((packed));
static void iso9660_read_block(uint32_t lba, uint32_t block, uint8_t* buf) {
    for (int i = 0; i < 4; ++i) read_sector(lba + block * 4 + i, buf + i * 512);
}
int iso9660_read_file(uint32_t lba, const char* filename, uint8_t* buf, uint32_t max_size) {
    uint8_t pvd[2048];
    iso9660_read_block(lba, 16, pvd);
    if (pvd[0] != 1 || memcmp(pvd+1, "CD001", 5) != 0) return -1;
    uint32_t root_extent = *(uint32_t*)(pvd+156);
    uint32_t root_size = *(uint32_t*)(pvd+164);
    uint8_t dir[2048];
    iso9660_read_block(lba, root_extent, dir);
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
            uint32_t to_read = (f_size < max_size) ? f_size : max_size;
            for (uint32_t i = 0; i < to_read; i += 2048) {
                uint32_t chunk = (to_read - i > 2048) ? 2048 : (to_read - i);
                iso9660_read_block(lba, f_extent + (i / 2048), buf + i);
            }
            return 0;
        }
        off += rec->len;
    }
    return -1;
} 