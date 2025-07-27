#include "iso9660.h"
#include "compat.h"
#include <stdint.h>
#include <string.h>
extern void read_sector(uint32_t lba, uint8_t* buf);
static void iso9660_read_block(uint32_t lba, uint32_t block, uint8_t* buf) {
    for (int i = 0; i < 4; ++i) read_sector(lba + block * 4 + i, buf + i * 512);
}
static int iso9660_stricmp(const char* a, const char* b, int n) {
    for (int i = 0; i < n; ++i) {
        char ca = a[i], cb = b[i];
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return 1;
        if (ca == 0) break;
    }
    return 0;
}
static int iso9660_find_entry(uint32_t lba, uint32_t extent, uint32_t size, const char* path, uint32_t* out_extent, uint32_t* out_size) {
    char part[256];
    const char* slash = strchr(path, '/');
    int partlen = slash ? (slash - path) : strlen(path);
    memcpy(part, path, partlen); part[partlen] = 0;
    uint8_t dir[2048];
    for (uint32_t off = 0; off < size;) {
        iso9660_read_block(lba, extent + (off / 2048), dir);
        uint32_t block_off = 0;
        while (block_off < 2048) {
            uint8_t len = dir[block_off];
            if (len == 0) break;
            uint8_t nlen = dir[block_off+32];
            char name[256];
            memcpy(name, dir+block_off+33, nlen); name[nlen] = 0;
            if (iso9660_stricmp(name, part, nlen) == 0) {
                uint32_t e = *(uint32_t*)(dir+block_off+2);
                uint32_t s = *(uint32_t*)(dir+block_off+10);
                uint8_t flags = dir[block_off+25];
                if (slash) {
                    if (flags & 2) {
                        if (iso9660_find_entry(lba, e, s, slash+1, out_extent, out_size) == 0) return 0;
                    }
                } else {
                    *out_extent = e;
                    *out_size = s;
                    return 0;
                }
            }
            block_off += len;
        }
        off += 2048;
    }
    return -1;
}
int iso9660_read_file(uint32_t lba, const char* path, uint8_t* buf, uint32_t max_size) {
    uint8_t pvd[2048];
    iso9660_read_block(lba, 16, pvd);
    if (pvd[0] != 1 || memcmp(pvd+1, "CD001", 5) != 0) return -1;
    uint32_t root_extent = *(uint32_t*)(pvd+156);
    uint32_t root_size = *(uint32_t*)(pvd+164);
    uint32_t f_extent, f_size;
    if (iso9660_find_entry(lba, root_extent, root_size, path, &f_extent, &f_size) != 0) return -1;
    uint32_t to_read = (f_size < max_size) ? f_size : max_size;
    for (uint32_t i = 0; i < to_read; i += 2048) {
        uint32_t chunk = (to_read - i > 2048) ? 2048 : (to_read - i);
        iso9660_read_block(lba, f_extent + (i / 2048), buf + i);
    }
    return 0;
} 