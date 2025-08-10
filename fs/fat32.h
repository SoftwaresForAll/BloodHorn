#ifndef BLOODHORN_FAT32_H
#define BLOODHORN_FAT32_H

#include <stdint.h>
#include <stdbool.h>
#include "compat.h"
#include "fs_mount.h"

// FAT32 Boot Sector (BPB)
struct fat32_bootsector {
    uint8_t     jump[3];
    uint8_t     oem[8];
    uint16_t    bytes_per_sector;
    uint8_t     sectors_per_cluster;
    uint16_t    reserved_sectors;
    uint8_t     num_fats;
    uint16_t    root_entries;
    uint16_t    total_sectors_16;
    uint8_t     media_type;
    uint16_t    sectors_per_fat_16;
    uint16_t    sectors_per_track;
    uint16_t    num_heads;
    uint32_t    hidden_sectors;
    uint32_t    total_sectors_32;
    
    // Extended FAT32 fields
    uint32_t    sectors_per_fat_32;
    uint16_t    ext_flags;
    uint16_t    fs_version;
    uint32_t    root_cluster;
    uint16_t    fs_info_sector;
    uint16_t    backup_boot_sector;
    uint8_t     reserved[12];
    uint8_t     drive_number;
    uint8_t     reserved1;
    uint8_t     boot_signature;
    uint32_t    volume_id;
    uint8_t     volume_label[11];
    uint8_t     fs_type[8];
    uint8_t     boot_code[420];
    uint16_t    boot_signature_55aa;
} __attribute__((packed));

// Directory entry attributes
#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20
#define ATTR_LONG_NAME  (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

// Directory entry (32 bytes)
struct fat32_dirent {
    uint8_t     name[11];
    uint8_t     attr;
    uint8_t     nt_res;
    uint8_t     crt_time_tenth;
    uint16_t    crt_time;
    uint16_t    crt_date;
    uint16_t    last_access_date;
    uint16_t    first_cluster_hi;
    uint16_t    write_time;
    uint16_t    write_date;
    uint16_t    first_cluster_lo;
    uint32_t    file_size;
} __attribute__((packed));

// FAT32 private data structure
typedef struct {
    uint32_t lba;                   // Starting LBA of partition
    struct fat32_bootsector bs;     // Boot sector
    uint32_t fat_begin_lba;         // Starting LBA of first FAT
    uint32_t cluster_begin_lba;     // Starting LBA of data clusters
    uint32_t root_dir_first_cluster; // First cluster of root directory
    uint32_t bytes_per_cluster;     // Bytes per cluster
    uint32_t total_clusters;        // Total number of data clusters
} fat32_private_t;

// FAT32 filesystem operations
extern const fs_operations_t fat32_ops;

// FAT32 filesystem type
extern const filesystem_t fat32_fs;

// Helper functions
uint32_t fat32_get_cluster(fat32_private_t *priv, uint32_t cluster);
int fat32_read_cluster(fat32_private_t *priv, uint32_t cluster, uint8_t *buffer);
int fat32_find_file(fat32_private_t *priv, const char *path, uint32_t *cluster, uint32_t *size);

#endif // BLOODHORN_FAT32_H