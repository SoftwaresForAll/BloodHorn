#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "compat.h"
#include "fat32.h"

// External disk I/O function
extern void read_sector(uint32_t lba, uint8_t* buf);

// FAT32 filesystem operations implementation
static int fat32_read(mount_point_t *mp, const char *path, uint8_t *buf, uint32_t size, uint32_t offset) {
    fat32_private_t *priv = (fat32_private_t *)mp->private_data;
    uint32_t cluster, file_size;
    
    if (fat32_find_file(priv, path, &cluster, &file_size) != 0) {
        return -1; // File not found
    }
    
    if (offset >= file_size) {
        return 0; // Read past end of file
    }
    
    if (offset + size > file_size) {
        size = file_size - offset; // Adjust size to not read past file end
    }
    
    // Calculate starting position
    uint32_t bytes_read = 0;
    uint32_t cluster_offset = offset / priv->bytes_per_cluster;
    uint32_t offset_in_cluster = offset % priv->bytes_per_cluster;
    
    // Skip to the starting cluster
    for (uint32_t i = 0; i < cluster_offset; i++) {
        cluster = fat32_get_cluster(priv, cluster);
        if (cluster >= 0x0FFFFFF8) {
            return -1; // Invalid cluster chain
        }
    }
    
    // Read data cluster by cluster
    while (bytes_read < size && cluster < 0x0FFFFFF8) {
        uint8_t *cluster_buf = (uint8_t *)malloc(priv->bytes_per_cluster);
        if (!cluster_buf) return -1;
        
        // Read the entire cluster
        if (fat32_read_cluster(priv, cluster, cluster_buf) != 0) {
            free(cluster_buf);
            return -1;
        }
        
        // Calculate how much to copy from this cluster
        uint32_t to_copy = priv->bytes_per_cluster - offset_in_cluster;
        if (bytes_read + to_copy > size) {
            to_copy = size - bytes_read;
        }
        
        // Copy the data
        memcpy(buf + bytes_read, cluster_buf + offset_in_cluster, to_copy);
        bytes_read += to_copy;
        
        free(cluster_buf);
        
        // Move to next cluster
        cluster = fat32_get_cluster(priv, cluster);
        offset_in_cluster = 0; // For subsequent clusters
    }
    
    return bytes_read;
}

static int fat32_read_dir(mount_point_t *mp, const char *path, char *buffer, uint32_t size) {
    fat32_private_t *priv = (fat32_private_t *)mp->private_data;
    uint32_t cluster, dir_size;
    
    if (fat32_find_file(priv, path, &cluster, &dir_size) != 0) {
        return -1; // Directory not found
    }
    
    uint32_t offset = 0;
    uint8_t *cluster_buf = (uint8_t *)malloc(priv->bytes_per_cluster);
    if (!cluster_buf) return -1;
    
    // Read directory entries cluster by cluster
    while (cluster < 0x0FFFFFF8) {
        if (fat32_read_cluster(priv, cluster, cluster_buf) != 0) {
            free(cluster_buf);
            return -1;
        }
        
        // Process directory entries in this cluster
        struct fat32_dirent *dent = (struct fat32_dirent *)cluster_buf;
        for (uint32_t i = 0; i < priv->bytes_per_cluster; i += sizeof(struct fat32_dirent), dent++) {
            // Check for end of directory
            if (dent->name[0] == 0x00) {
                free(cluster_buf);
                return offset; // End of directory
            }
            
            // Skip deleted or long name entries
            if (dent->name[0] == 0xE5 || (dent->attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) {
                continue;
            }
            
            // Convert 8.3 name to normal format
            char name[13];
            int pos = 0;
            
            // Copy name part (8 chars)
            for (int i = 0; i < 8 && dent->name[i] != ' '; i++) {
                name[pos++] = dent->name[i];
            }
            
            // Add extension if present
            if (dent->name[8] != ' ') {
                name[pos++] = '.';
                for (int i = 8; i < 11 && dent->name[i] != ' '; i++) {
                    name[pos++] = dent->name[i];
                }
            }
            name[pos] = '\0';
            
            // Add entry to buffer if there's space
            int needed = pos + 2; // Name + space + newline
            if (offset + needed > size) {
                free(cluster_buf);
                return offset; // Out of buffer space
            }
            
            memcpy(buffer + offset, name, pos);
            offset += pos;
            buffer[offset++] = '\n';
        }
        
        // Move to next cluster
        cluster = fat32_get_cluster(priv, cluster);
    }
    
    free(cluster_buf);
    return offset;
}

static int fat32_get_info(mount_point_t *mp, const char *path, uint32_t *size, bool *is_dir) {
    fat32_private_t *priv = (fat32_private_t *)mp->private_data;
    uint32_t cluster;
    
    if (fat32_find_file(priv, path, &cluster, size) != 0) {
        return -1; // File not found
    }
    
    // For simplicity, we'll say it's a directory if it has the directory attribute
    // In a real implementation, you'd need to check the actual directory entry
    *is_dir = (cluster & ATTR_DIRECTORY) != 0;
    
    return 0;
}

// FAT32 filesystem operations structure
const fs_operations_t fat32_ops = {
    .read = fat32_read,
    .write = NULL, // Read-only for now
    .list_dir = fat32_read_dir,
    .get_info = fat32_get_info,
};

// Helper function to read a FAT entry
uint32_t fat32_get_cluster(fat32_private_t *priv, uint32_t cluster) {
    if (cluster >= 0x0FFFFFF8) {
        return 0x0FFFFFFF; // End of cluster chain
    }
    
    uint32_t fat_offset = cluster * 4; // 32-bit FAT entries
    uint32_t fat_sector = priv->fat_begin_lba + (fat_offset / priv->bs.bytes_per_sector);
    uint32_t entry_offset = fat_offset % priv->bs.bytes_per_sector;
    
    uint8_t sector[512];
    read_sector(fat_sector, sector);
    
    // Get next cluster number (mask off high 4 bits)
    return (*(uint32_t*)(sector + entry_offset)) & 0x0FFFFFFF;
}

// Read an entire cluster
int fat32_read_cluster(fat32_private_t *priv, uint32_t cluster, uint8_t *buffer) {
    if (cluster < 2 || cluster >= priv->total_clusters + 2) {
        return -1; // Invalid cluster number
    }
    
    // Calculate first sector of cluster
    uint32_t first_sector = priv->cluster_begin_lba + 
                           ((cluster - 2) * priv->bs.sectors_per_cluster);
    
    // Read all sectors in the cluster
    for (uint32_t i = 0; i < priv->bs.sectors_per_cluster; i++) {
        read_sector(first_sector + i, buffer + (i * priv->bs.bytes_per_sector));
    }
    
    return 0;
}

// Find a file in the filesystem
int fat32_find_file(fat32_private_t *priv, const char *path, uint32_t *cluster, uint32_t *size) {
    // Start at root directory
    uint32_t current_cluster = priv->root_dir_first_cluster;
    
    // Skip leading slashes
    while (*path == '/') path++;
    
    // Handle root directory
    if (*path == '\0') {
        *cluster = current_cluster;
        *size = 0; // Directories have size 0 in FAT32
        return 0;
    }
    
    // Split path into components
    char component[256];
    const char *next = strchr(path, '/');
    size_t len = next ? (size_t)(next - path) : strlen(path);
    
    if (len >= sizeof(component)) {
        return -1; // Component too long
    }
    
    strncpy(component, path, len);
    component[len] = '\0';
    
    // Convert component to 8.3 format
    char fatname[12];
    memset(fatname, ' ', 11);
    fatname[11] = '\0';
    
    const char *dot = strchr(component, '.');
    if (dot) {
        // Has extension
        size_t name_len = dot - component;
        if (name_len > 8) name_len = 8;
        
        memcpy(fatname, component, name_len);
        
        const char *ext = dot + 1;
        size_t ext_len = strlen(ext);
        if (ext_len > 3) ext_len = 3;
        
        memcpy(fatname + 8, ext, ext_len);
    } else {
        // No extension
        size_t name_len = strlen(component);
        if (name_len > 8) name_len = 8;
        memcpy(fatname, component, name_len);
    }
    
    // Make uppercase
    for (int i = 0; i < 11; i++) {
        if (fatname[i] >= 'a' && fatname[i] <= 'z') {
            fatname[i] = fatname[i] - 'a' + 'A';
        }
    }
    
    // Search directory for the component
    uint8_t *cluster_buf = (uint8_t *)malloc(priv->bytes_per_cluster);
    if (!cluster_buf) return -1;
    
    while (current_cluster < 0x0FFFFFF8) {
        if (fat32_read_cluster(priv, current_cluster, cluster_buf) != 0) {
            free(cluster_buf);
            return -1;
        }
        
        struct fat32_dirent *dent = (struct fat32_dirent *)cluster_buf;
        for (uint32_t i = 0; i < priv->bytes_per_cluster; i += sizeof(struct fat32_dirent), dent++) {
            // Check for end of directory
            if (dent->name[0] == 0x00) {
                free(cluster_buf);
                return -1; // Not found
            }
            
            // Skip deleted or long name entries
            if (dent->name[0] == 0xE5 || (dent->attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) {
                continue;
            }
            
            // Compare names
            if (memcmp(dent->name, fatname, 11) == 0) {
                // Found the entry
                *cluster = (dent->first_cluster_hi << 16) | dent->first_cluster_lo;
                *size = dent->file_size;
                
                // If this is the last component, return success
                if (!next) {
                    free(cluster_buf);
                    return 0;
                }
                
                // Otherwise, continue with next component
                free(cluster_buf);
                return fat32_find_file(priv, next + 1, cluster, size);
            }
        }
        
        // Move to next cluster in chain
        current_cluster = fat32_get_cluster(priv, current_cluster);
    }
    
    free(cluster_buf);
    return -1; // Not found
}

// Mount function for FAT32
static void *fat32_mount(uint32_t lba, void *opts) {
    fat32_private_t *priv = (fat32_private_t *)malloc(sizeof(fat32_private_t));
    if (!priv) return NULL;
    
    // Read boot sector
    read_sector(lba, (uint8_t *)&priv->bs);
    
    // Verify FAT32 signature
    if (priv->bs.boot_signature_55aa != 0xAA55) {
        free(priv);
        return NULL;
    }
    
    // Initialize private data
    priv->lba = lba;
    priv->fat_begin_lba = lba + priv->bs.reserved_sectors;
    priv->bytes_per_cluster = priv->bs.bytes_per_sector * priv->bs.sectors_per_cluster;
    
    // Calculate data area start
    uint32_t fat_size = priv->bs.sectors_per_fat_32 ? 
                       priv->bs.sectors_per_fat_32 : 
                       priv->bs.sectors_per_fat_16;
    
    priv->cluster_begin_lba = priv->fat_begin_lba + (priv->bs.num_fats * fat_size);
    priv->root_dir_first_cluster = priv->bs.root_cluster;
    
    // Calculate total clusters
    uint32_t data_sectors = priv->bs.total_sectors_32 ? 
                           priv->bs.total_sectors_32 : 
                           priv->bs.total_sectors_16;
    
    data_sectors -= (priv->cluster_begin_lba - lba);
    priv->total_clusters = data_sectors / priv->bs.sectors_per_cluster;
    
    return priv;
}

// Unmount function for FAT32
static void fat32_unmount(void *private_data) {
    if (private_data) {
        free(private_data);
    }
}

// Detect if a partition is FAT32
static bool fat32_detect(uint32_t lba) {
    struct fat32_bootsector bs;
    read_sector(lba, (uint8_t *)&bs);
    
    // Check FAT32 signature
    if (bs.boot_signature_55aa != 0xAA55) {
        return false;
    }
    
    // Check for FAT32 filesystem type
    if (memcmp(bs.fs_type, "FAT32   ", 8) != 0 &&
        memcmp(bs.fs_type, "FAT32   ", 8) != 0) {
        return false;
    }
    
    // Check for valid sector size (power of 2 between 512 and 4096)
    if ((bs.bytes_per_sector & (bs.bytes_per_sector - 1)) != 0 ||
        bs.bytes_per_sector < 512 || bs.bytes_per_sector > 4096) {
        return false;
    }
    
    // Check for valid sectors per cluster (power of 2)
    if ((bs.sectors_per_cluster & (bs.sectors_per_cluster - 1)) != 0 ||
        bs.sectors_per_cluster == 0) {
        return false;
    }
    
    return true;
}

// FAT32 filesystem type structure
const filesystem_t fat32_fs = {
    .name = "fat32",
    .ops = &fat32_ops,
    .detect = fat32_detect,
    .mount = fat32_mount,
    .unmount = fat32_unmount,
};