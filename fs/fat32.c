#include <stdint.h>
#include <string.h>
#include "fat32.h"

extern void read_sector(uint32_t lba, uint8_t* buf);

int fat32_read_bootsector(uint32_t lba, struct fat32_bootsector* bs) {
    uint8_t buf[512];
    read_sector(lba, buf);
    memcpy(bs, buf, sizeof(struct fat32_bootsector));
    return 0;
}

int fat32_find_file(uint32_t lba, const char* filename, uint32_t* cluster_out) {
    struct fat32_bootsector bs;
    fat32_read_bootsector(lba, &bs);
    uint32_t fat_offset = bs.reserved_sectors;
    uint32_t root_dir_sectors = ((bs.root_entries * 32) + (bs.bytes_per_sector - 1)) / bs.bytes_per_sector;
    uint32_t first_data_sector = fat_offset + (bs.num_fats * bs.sectors_per_fat) + root_dir_sectors;
    uint32_t root_dir_start = lba + fat_offset + (bs.num_fats * bs.sectors_per_fat);
    uint8_t dir[512];
    read_sector(root_dir_start, dir);
    for (int i = 0; i < 512; i += 32) {
        if (dir[i] == 0) break;
        if (dir[i] == 0xE5) continue;
        if ((dir[i+11] & 0x10) != 0) continue;
        char name[12];
        memcpy(name, &dir[i], 11);
        name[11] = 0;
        if (strcmp(name, filename) == 0) {
            uint32_t cluster = (dir[i+26] | (dir[i+27] << 8));
            *cluster_out = cluster;
            return 0;
        }
    }
    return -1;
}

int fat32_read_file(uint32_t lba, uint32_t cluster, uint8_t* buf, uint32_t max_size) {
    struct fat32_bootsector bs;
    fat32_read_bootsector(lba, &bs);
    uint32_t fat_offset = bs.reserved_sectors;
    uint32_t root_dir_sectors = ((bs.root_entries * 32) + (bs.bytes_per_sector - 1)) / bs.bytes_per_sector;
    uint32_t first_data_sector = fat_offset + (bs.num_fats * bs.sectors_per_fat) + root_dir_sectors;
    uint32_t data_sector = lba + first_data_sector + (cluster - 2) * bs.sectors_per_cluster;
    read_sector(data_sector, buf);
    return 0;
} 