#ifndef BLOODHORN_FAT32_H
#define BLOODHORN_FAT32_H
#include <stdint.h>

struct fat32_bootsector {
    uint8_t jump[3];
    uint8_t oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entries;
    uint16_t total_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t large_sectors;
};

int fat32_read_bootsector(uint32_t lba, struct fat32_bootsector* bs);
int fat32_find_file(uint32_t lba, const char* filename, uint32_t* cluster_out);
int fat32_read_file(uint32_t lba, uint32_t cluster, uint8_t* buf, uint32_t max_size);

#endif // BLOODHORN_FAT32_H 