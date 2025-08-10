#include "iso9660.h"
#include "compat.h"
#include "mm.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

// External disk I/O function
extern int disk_read(void *buf, uint32_t lba, uint32_t count);

// Global filesystem instance
static const filesystem_t iso9660_fs = {
    .name = "iso9660",
    .detect = iso9660_detect,
    .mount = iso9660_mount,
    .unmount = iso9660_unmount,
    .ops = &iso9660_ops
};

// Filesystem operations
static const fs_operations_t iso9660_ops = {
    .read_file = iso9660_read_file,
    .list_dir = iso9660_list_dir,
    .get_info = iso9660_get_info,
    .find_file = iso9660_find_file_fs
};

// Internal helper functions
static int iso9660_read_blocks(iso9660_private_t *priv, uint32_t block, uint32_t count, void *buf) {
    uint32_t lba = priv->lba + block * (priv->block_size / 512);
    return disk_read(buf, lba, count * (priv->block_size / 512));
}

static int iso9660_read_block(iso9660_private_t *priv, uint32_t block_num, void *buf) {
    return iso9660_read_blocks(priv, block_num, 1, buf);
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

static int iso9660_read_volume_descriptor(iso9660_private_t *priv) {
    // Read volume descriptor sequence starting at sector 16
    uint8_t buffer[2048];
    
    // Try up to 32 volume descriptors (should be enough)
    for (int i = 16; i < 48; i++) {
        if (disk_read(buffer, priv->lba + i, 1) != 0) {
            return -1; // Read error
        }
        
        // Check for primary volume descriptor (type 1)
        if (buffer[0] == 0x01) {
            // Found primary volume descriptor
            memcpy(&priv->pvd, buffer, sizeof(struct iso_volume_descriptor));
            priv->block_size = priv->pvd.logical_block_size;
            return 0;
        }
        // Check for volume descriptor set terminator (type 255)
        else if (buffer[0] == 0xFF) {
            break; // End of volume descriptors
        }
    }
    
    return -1; // Primary volume descriptor not found
}

static int iso9660_read_path_table(iso9660_private_t *priv) {
    // Read the path table (simplified - just read the first one)
    uint32_t path_table_lba = priv->pvd.path_table_l;
    priv->path_table_size = priv->pvd.path_table_size;
    
    // Allocate memory for path table
    priv->path_table = (uint8_t *)kmalloc(priv->path_table_size);
    if (!priv->path_table) {
        return -1; // Out of memory
    }
    
    // Read path table
    return iso9660_read_blocks(priv, path_table_lba, 
                              (priv->path_table_size + priv->block_size - 1) / priv->block_size, 
                              priv->path_table);
}

// Public interface implementation
int iso9660_detect(uint32_t lba) {
    uint8_t buffer[2048];
    
    // Try to read the first sector of the volume descriptor
    if (disk_read(buffer, lba + 16, 1) != 0) {
        return 0; // Read error
    }
    
    // Check for ISO9660 signature ("CD001" at offset 1)
    if (buffer[0] == 0x01 && 
        buffer[1] == 'C' && 
        buffer[2] == 'D' && 
        buffer[3] == '0' && 
        buffer[4] == '0' && 
        buffer[5] == '1') {
        return 1; // Valid ISO9660 filesystem
    }
    
    return 0; // Not an ISO9660 filesystem
}

void *iso9660_mount(uint32_t lba) {
    // Allocate and initialize private data
    iso9660_private_t *priv = (iso9660_private_t *)kmalloc(sizeof(iso9660_private_t));
    if (!priv) {
        return NULL; // Out of memory
    }
    
    // Initialize private data
    memset(priv, 0, sizeof(iso9660_private_t));
    priv->lba = lba + 16; // Skip the first 16 sectors (system area)
    
    // Read volume descriptor
    if (iso9660_read_volume_descriptor(priv) != 0) {
        kfree(priv);
        return NULL; // Failed to read volume descriptor
    }
    
    // Read path table (optional, but useful for faster lookups)
    if (iso9660_read_path_table(priv) != 0) {
        // Not fatal, we can still work without the path table
        if (priv->path_table) {
            kfree(priv->path_table);
            priv->path_table = NULL;
        }
    }
    
    return priv;
}

void iso9660_unmount(void *private_data) {
    if (!private_data) return;
    
    iso9660_private_t *priv = (iso9660_private_t *)private_data;
    
    // Free path table if allocated
    if (priv->path_table) {
        kfree(priv->path_table);
    }
    
    // Free private data
    kfree(priv);
}

int iso9660_find_entry(iso9660_private_t *priv, uint32_t extent, uint32_t size, const char* path, uint32_t* out_extent, uint32_t* out_size) {
    char part[256];
    const char* slash = strchr(path, '/');
    int partlen = slash ? (slash - path) : strlen(path);
    memcpy(part, path, partlen); 
    part[partlen] = 0;
    
    uint8_t *dir = (uint8_t *)kmalloc(priv->block_size);
    if (!dir) {
        return -1; // Out of memory
    }
    
    // Read directory entries
    for (uint32_t off = 0; off < size; off += priv->block_size) {
        uint32_t block = extent + (off / priv->block_size);
        if (iso9660_read_block(priv, block, dir) != 0) {
            kfree(dir);
            return -1; // Read error
        }
        
        uint32_t block_off = 0;
        while (block_off < priv->block_size) {
            struct iso_directory_record *record = (struct iso_directory_record *)(dir + block_off);
            
            // Check for end of directory
            if (record->length == 0) {
                block_off += (priv->block_size - block_off);
                continue;
            }
            
            // Skip current and parent directory entries
            if (record->name_len == 1 && (record->name[0] == 0 || record->name[0] == 1)) {
                block_off += record->length;
                continue;
            }
            
            // Compare filename
            if (record->name_len == partlen && 
                iso9660_stricmp(record->name, part, partlen) == 0) {
                
                uint32_t e = record->extent_l;
                uint32_t s = record->data_length_l;
                uint8_t flags = record->file_flags;
                
                if (slash) {
                    // Recurse into subdirectory
                    if (flags & 0x02) { // Directory flag
                        int result = iso9660_find_entry(priv, e, s, slash + 1, out_extent, out_size);
                        kfree(dir);
                        return result;
                    }
                } else {
                    // Found the file
                    *out_extent = e;
                    *out_size = s;
                    kfree(dir);
                    return 0;
                }
            }
            
            block_off += record->length;
            if (record->length == 0) break; // Prevent infinite loop
        }
    }
    
    kfree(dir);
    return -1; // Not found
}

// Filesystem operation implementations
static int iso9660_read_file(void *private_data, const char *path, void *buf, 
                            uint32_t size, uint32_t offset) {
    if (!private_data || !path || !buf) {
        return -1;
    }
    
    iso9660_private_t *priv = (iso9660_private_t *)private_data;
    uint32_t extent, file_size;
    
    // Find the file
    if (iso9660_find_entry(priv, priv->pvd.root_directory_record[2], 
                          *(uint32_t*)(priv->pvd.root_directory_record + 10),
                          path, &extent, &file_size) != 0) {
        return -1; // File not found
    }
    
    // Adjust read size if needed
    if (offset >= file_size) {
        return 0; // Read nothing, offset beyond file size
    }
    
    if (offset + size > file_size) {
        size = file_size - offset; // Adjust size to not read beyond file
    }
    
    // Calculate blocks to read
    uint32_t start_block = extent + (offset / priv->block_size);
    uint32_t end_block = extent + ((offset + size - 1) / priv->block_size);
    uint32_t blocks_to_read = end_block - start_block + 1;
    
    // Allocate buffer for reading
    uint8_t *block_buf = (uint8_t *)kmalloc(blocks_to_read * priv->block_size);
    if (!block_buf) {
        return -1; // Out of memory
    }
    
    // Read the blocks
    if (iso9660_read_blocks(priv, start_block, blocks_to_read, block_buf) != 0) {
        kfree(block_buf);
        return -1; // Read error
    }
    
    // Copy the requested data to the output buffer
    uint32_t block_offset = offset % priv->block_size;
    memcpy(buf, block_buf + block_offset, size);
    
    kfree(block_buf);
    return size;
}

static int iso9660_list_dir(void *private_data, const char *path, 
                           fs_dirent_t *entries, uint32_t max_entries) {
    if (!private_data || !path) {
        return -1;
    }
    
    iso9660_private_t *priv = (iso9660_private_t *)private_data;
    uint32_t extent, size;
    
    // Find the directory
    if (iso9660_find_entry(priv, priv->pvd.root_directory_record[2], 
                          *(uint32_t*)(priv->pvd.root_directory_record + 10),
                          path, &extent, &size) != 0) {
        return -1; // Directory not found
    }
    
    uint8_t *dir = (uint8_t *)kmalloc(priv->block_size);
    if (!dir) {
        return -1; // Out of memory
    }
    
    uint32_t count = 0;
    
    // Read directory entries
    for (uint32_t off = 0; off < size && count < max_entries; off += priv->block_size) {
        uint32_t block = extent + (off / priv->block_size);
        if (iso9660_read_block(priv, block, dir) != 0) {
            kfree(dir);
            return -1; // Read error
        }
        
        uint32_t block_off = 0;
        while (block_off < priv->block_size && count < max_entries) {
            struct iso_directory_record *record = (struct iso_directory_record *)(dir + block_off);
            
            // Check for end of directory
            if (record->length == 0) {
                block_off += (priv->block_size - block_off);
                continue;
            }
            
            // Skip current and parent directory entries
            if (record->name_len == 1 && (record->name[0] == 0 || record->name[0] == 1)) {
                block_off += record->length;
                continue;
            }
            
            // Add entry to the list
            if (entries && count < max_entries) {
                entries[count].inode = record->extent_l; // Use extent as inode number
                
                // Copy name (ensure null termination)
                int name_len = record->name_len;
                if (name_len >= FS_MAX_FILENAME) {
                    name_len = FS_MAX_FILENAME - 1;
                }
                memcpy(entries[count].name, record->name, name_len);
                entries[count].name[name_len] = '\0';
                entries[count].name_len = name_len;
                
                // Set file type
                if (record->file_flags & 0x02) {
                    entries[count].file_type = FS_FILE_DIRECTORY;
                } else {
                    entries[count].file_type = FS_FILE_REGULAR;
                }
                
                count++;
            }
            
            block_off += record->length;
            if (record->length == 0) break; // Prevent infinite loop
        }
    }
    
    kfree(dir);
    return count;
}

static int iso9660_get_info(void *private_data, const char *path, fs_file_info_t *info) {
    if (!private_data || !path || !info) {
        return -1;
    }
    
    iso9660_private_t *priv = (iso9660_private_t *)private_data;
    uint32_t extent, size;
    
    // Find the file/directory
    if (iso9660_find_entry(priv, priv->pvd.root_directory_record[2], 
                          *(uint32_t*)(priv->pvd.root_directory_record + 10),
                          path, &extent, &size) != 0) {
        return -1; // Not found
    }
    
    // Read the directory record for the file
    uint8_t *dir = (uint8_t *)kmalloc(priv->block_size);
    if (!dir) {
        return -1; // Out of memory
    }
    
    // Read the directory block containing the entry
    uint32_t dir_block = extent / (priv->block_size / 256); // Approximate block
    if (iso9660_read_block(priv, dir_block, dir) != 0) {
        kfree(dir);
        return -1; // Read error
    }
    
    // Find the directory record (simplified)
    struct iso_directory_record *record = NULL;
    for (uint32_t i = 0; i < priv->block_size - sizeof(struct iso_directory_record); i++) {
        struct iso_directory_record *r = (struct iso_directory_record *)(dir + i);
        if (r->length == 0) break;
        if (r->extent_l == extent && r->data_length_l == size) {
            record = r;
            break;
        }
    }
    
    if (!record) {
        kfree(dir);
        return -1; // Record not found
    }
    
    // Fill in the file info
    memset(info, 0, sizeof(fs_file_info_t));
    info->size = size;
    
    // Set file type
    if (record->file_flags & 0x02) {
        info->type = FS_FILE_DIRECTORY;
    } else {
        info->type = FS_FILE_REGULAR;
    }
    
    // Set mode (simplified)
    info->mode = 0444; // Read-only by default
    if (info->type == FS_FILE_DIRECTORY) {
        info->mode |= 0111; // Add execute permission for directories
    }
    
    // Set timestamps (simplified)
    // ISO9660 stores dates in a custom format, this is a simplified conversion
    // In a real implementation, you would properly parse the ISO9660 date format
    info->atime = 0; // Not available
    info->mtime = 0; // Would parse from record->date
    info->ctime = 0; // Not available
    
    kfree(dir);
    return 0;
}

static int iso9660_find_file_fs(void *private_data, const char *path, uint32_t *inode_out) {
    if (!private_data || !path || !inode_out) {
        return -1;
    }
    
    iso9660_private_t *priv = (iso9660_private_t *)private_data;
    uint32_t extent, size;
    
    // Find the file/directory
    if (iso9660_find_entry(priv, priv->pvd.root_directory_record[2], 
                          *(uint32_t*)(priv->pvd.root_directory_record + 10),
                          path, &extent, &size) != 0) {
        return -1; // Not found
    }
    
    // Use the extent as an inode number (simplified)
    *inode_out = extent;
    return 0;
}

// Module initialization
void iso9660_init(void) {
    fs_register(&iso9660_fs);
}