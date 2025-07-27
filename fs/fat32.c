#include <stdint.h>
#include "compat.h"
#include <string.h>
#include "fat32.h"

extern void read_sector(uint32_t lba, uint8_t* buf);

static uint32_t fat32_get_fat_entry(uint32_t lba, const struct fat32_bootsector* bs, uint32_t cluster) {
    uint32_t fat_offset = bs->reserved_sectors;
    uint32_t fat_sector = fat_offset + ((cluster * 4) / bs->bytes_per_sector);
    uint8_t sector[512];
    read_sector(lba + fat_sector, sector);
    uint32_t entry_offset = (cluster * 4) % bs->bytes_per_sector;
    return (*(uint32_t*)&sector[entry_offset]) & 0x0FFFFFFF;
}

int fat32_read_bootsector(uint32_t lba, struct fat32_bootsector* bs) {
    uint8_t buf[512];
    read_sector(lba, buf);
    memcpy(bs, buf, sizeof(struct fat32_bootsector));
    return 0;
}

static int fat32_compare_name(const char* entry, const char* name) {
    char fatname[12];
    memset(fatname, ' ', 11); fatname[11] = 0;
    int i = 0;
    for (; name[i] && i < 11; ++i) fatname[i] = name[i];
    return strncmp(entry, fatname, 11) == 0;
}

int fat32_find_file(uint32_t lba, const char* filename, uint32_t* cluster_out) {
    struct fat32_bootsector bs;
    fat32_read_bootsector(lba, &bs);
    uint32_t root_cluster = bs.root_cluster;
    uint8_t sector[512];
    uint32_t cluster = root_cluster;
    while (cluster < 0x0FFFFFF8) {
        for (uint32_t s = 0; s < bs.sectors_per_cluster; ++s) {
            uint32_t data_sector = lba + bs.reserved_sectors + (bs.num_fats * bs.sectors_per_fat) + ((cluster - 2) * bs.sectors_per_cluster) + s;
            read_sector(data_sector, sector);
            for (int i = 0; i < 512; i += 32) {
                if (sector[i] == 0) break;
                if (sector[i] == 0xE5) continue;
                if ((sector[i+11] & 0x08) != 0) continue;
                if (fat32_compare_name((char*)&sector[i], filename)) {
                    *cluster_out = (sector[i+26] | (sector[i+27] << 8)) | ((sector[i+20] | (sector[i+21] << 8)) << 16);
                    return 0;
                }
            }
        }
        cluster = fat32_get_fat_entry(lba, &bs, cluster);
    }
    return -1;
}

int fat32_read_file(uint32_t lba, uint32_t cluster, uint8_t* buf, uint32_t max_size) {
    struct fat32_bootsector bs;
    fat32_read_bootsector(lba, &bs);
    uint32_t bytes_per_cluster = bs.sectors_per_cluster * bs.bytes_per_sector;
    uint32_t total_read = 0;
    while (cluster < 0x0FFFFFF8 && total_read < max_size) {
        for (uint32_t s = 0; s < bs.sectors_per_cluster && total_read < max_size; ++s) {
            uint32_t data_sector = lba + bs.reserved_sectors + (bs.num_fats * bs.sectors_per_fat) + ((cluster - 2) * bs.sectors_per_cluster) + s;
            read_sector(data_sector, buf + total_read);
            total_read += bs.bytes_per_sector;
        }
        cluster = fat32_get_fat_entry(lba, &bs, cluster);
    }
    return 0;
} 