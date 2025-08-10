#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "compat.h"
#include "ext2.h"
#include "fs_common.h"
#include "mm.h"

// External disk I/O function
extern int disk_read(void *buf, uint32_t lba, uint32_t count);

// Global filesystem instance
static const filesystem_t ext2_fs = {
    .name = "ext2",
    .detect = ext2_detect,
    .mount = ext2_mount,
    .unmount = ext2_unmount,
    .ops = &ext2_ops
};

// Filesystem operations
static const fs_operations_t ext2_ops = {
    .read_file = ext2_read_file,
    .list_dir = ext2_list_dir,
    .get_info = ext2_get_info,
    .find_file = ext2_find_file
};

// Internal helper functions
static int ext2_read_blocks(ext2_private_t *priv, uint32_t block, uint32_t count, void *buf) {
    uint32_t lba = priv->lba + block * (priv->block_size / 512);
    return disk_read(buf, lba, count * (priv->block_size / 512));
}

static int ext2_read_block(ext2_private_t *priv, uint32_t block_num, void *buf) {
    return ext2_read_blocks(priv, block_num, 1, buf);
}

static int ext2_read_superblock(ext2_private_t *priv) {
    // Superblock is at offset 1024 bytes (block 1 for 1024-byte blocks)
    uint32_t lba = priv->lba + (1024 / 512);
    return disk_read(&priv->sb, lba, 2); // Read 2 sectors (1024 bytes)
}

static int ext2_read_group_descriptors(ext2_private_t *priv) {
    // Calculate number of block group descriptors
    priv->group_count = (priv->sb.s_blocks_count + priv->blocks_per_group - 1) / priv->blocks_per_group;
    
    // Allocate memory for group descriptors
    uint32_t gd_size = priv->group_count * sizeof(struct ext2_group_desc);
    priv->gd = (struct ext2_group_desc *)kmalloc(gd_size);
    if (!priv->gd) {
        return -1; // Out of memory
    }
    
    // Read group descriptors (starts at first block after superblock)
    uint32_t gd_block = 1 + (priv->block_size > 1024 ? 1 : 0);
    return ext2_read_blocks(priv, gd_block, 
                           (gd_size + priv->block_size - 1) / priv->block_size, 
                           priv->gd);
}

// Public interface implementation
int ext2_detect(uint32_t lba) {
    struct ext2_superblock sb;
    
    // Read superblock
    if (disk_read(&sb, lba + (1024 / 512), 2) != 0) {
        return 0; // Read error
    }
    
    // Check magic number
    if (sb.s_magic != EXT2_SUPER_MAGIC) {
        return 0; // Not an ext2 filesystem
    }
    
    // Check if this is a valid ext2 filesystem
    if (sb.s_rev_level == 0 && sb.s_first_ino != 11) {
        return 0; // Invalid first inode
    }
    
    return 1; // Valid ext2 filesystem
}

void *ext2_mount(uint32_t lba) {
    // Allocate and initialize private data
    ext2_private_t *priv = (ext2_private_t *)kmalloc(sizeof(ext2_private_t));
    if (!priv) {
        return NULL; // Out of memory
    }
    
    // Initialize private data
    memset(priv, 0, sizeof(ext2_private_t));
    priv->lba = lba;
    
    // Read superblock
    if (ext2_read_superblock(priv) != 0) {
        kfree(priv);
        return NULL; // Failed to read superblock
    }
    
    // Verify superblock
    if (priv->sb.s_magic != EXT2_SUPER_MAGIC) {
        kfree(priv);
        return NULL; // Invalid superblock
    }
    
    // Calculate filesystem parameters
    priv->block_size = 1024 << priv->sb.s_log_block_size;
    priv->inode_size = priv->sb.s_inode_size < 128 ? 128 : priv->sb.s_inode_size;
    priv->blocks_per_group = priv->sb.s_blocks_per_group;
    priv->inodes_per_group = priv->sb.s_inodes_per_group;
    priv->inodes_per_block = priv->block_size / priv->inode_size;
    priv->desc_per_block = priv->block_size / sizeof(struct ext2_group_desc);
    
    // Read group descriptors
    if (ext2_read_group_descriptors(priv) != 0) {
        kfree(priv);
        return NULL; // Failed to read group descriptors
    }
    
    return priv;
}

void ext2_unmount(void *private_data) {
    if (!private_data) return;
    
    ext2_private_t *priv = (ext2_private_t *)private_data;
    
    // Free group descriptors
    if (priv->gd) {
        kfree(priv->gd);
    }
    
    // Free private data
    kfree(priv);
}

int ext2_read_inode(ext2_private_t *priv, uint32_t inode_num, struct ext2_inode *inode) {
    if (!priv || !inode || inode_num == 0) {
        return -1; // Invalid parameters
    }
    
    // Calculate which block group the inode is in
    uint32_t group = (inode_num - 1) / priv->inodes_per_group;
    if (group >= priv->group_count) {
        return -1; // Invalid inode number
    }
    
    // Get the inode table block for this group
    uint32_t inode_table_block = priv->gd[group].bg_inode_table;
    
    // Calculate the index of the inode in the inode table
    uint32_t index = (inode_num - 1) % priv->inodes_per_group;
    uint32_t block_offset = (index * priv->inode_size) / priv->block_size;
    uint32_t inode_offset = (index * priv->inode_size) % priv->block_size;
    
    // Read the block containing the inode
    uint8_t *block = (uint8_t *)kmalloc(priv->block_size);
    if (!block) {
        return -1; // Out of memory
    }
    
    if (ext2_read_block(priv, inode_table_block + block_offset, block) != 0) {
        kfree(block);
        return -1; // Read error
    }
    
    // Copy the inode data
    memcpy(inode, block + inode_offset, sizeof(struct ext2_inode));
    kfree(block);
    
    return 0;
}

int ext2_read_file(ext2_private_t *priv, uint32_t inode_num, uint8_t *buf, uint32_t max_size) {
    struct ext2_inode inode;
    if (ext2_read_inode(priv, inode_num, &inode) != 0) {
        return -1; // Failed to read inode
    }
    
    uint32_t file_size = inode.i_size;
    uint32_t to_read = (file_size < max_size) ? file_size : max_size;
    uint32_t read_bytes = 0;
    
    for (int i = 0; i < 12 && read_bytes < to_read; ++i) {
        if (inode.i_block[i] == 0) break;
        
        uint32_t chunk = (to_read - read_bytes > priv->block_size) ? priv->block_size : (to_read - read_bytes);
        if (ext2_read_block(priv, inode.i_block[i], buf + read_bytes) != 0) {
            return -1; // Read error
        }
        read_bytes += chunk;
    }
    
    return 0;
}

int ext2_find_file(ext2_private_t *priv, const char *filename, uint32_t *inode_out) {
    struct ext2_inode root_inode;
    if (ext2_read_inode(priv, 2, &root_inode) != 0) {
        return -1; // Failed to read root inode
    }
    
    uint32_t block_size = priv->block_size;
    uint8_t dir[block_size];
    if (ext2_read_block(priv, root_inode.i_block[0], dir) != 0) {
        return -1; // Read error
    }
    
    uint32_t offset = 0;
    while (offset < block_size) {
        uint32_t inode_num = *(uint32_t *)&dir[offset];
        uint16_t rec_len = *(uint16_t *)&dir[offset+4];
        uint8_t name_len = dir[offset+6];
        char name[256];
        memcpy(name, dir+offset+8, name_len); name[name_len] = 0;
        
        if (inode_num && strcmp(name, filename) == 0) {
            *inode_out = inode_num;
            return 0;
        }
        
        offset += rec_len;
        if (rec_len == 0) break;
    }
    
    return -1;
}

int ext2_list_dir(ext2_private_t *priv, const char *path, fs_dirent_t *entries, uint32_t max_entries) {
    uint32_t inode_num;
    struct ext2_inode inode;
    
    // Find the inode for the given path
    if (strcmp(path, "/") == 0) {
        inode_num = 2; // Root directory
    } else {
        if (ext2_find_file(priv, path, &inode_num) != 0) {
            return -1; // Path not found
        }
    }
    
    // Read the inode
    if (ext2_read_inode(priv, inode_num, &inode) != 0) {
        return -1; // Failed to read inode
    }
    
    // Check if it's a directory
    if ((inode.i_mode & EXT2_S_IFMT) != EXT2_S_IFDIR) {
        return -1; // Not a directory
    }
    
    uint32_t block_size = priv->block_size;
    uint8_t *block = (uint8_t *)kmalloc(block_size);
    if (!block) {
        return -1; // Out of memory
    }
    
    uint32_t entry_count = 0;
    
    // Read directory entries from direct blocks (simplified - only first 12 blocks)
    for (int i = 0; i < 12 && inode.i_block[i] != 0; i++) {
        if (ext2_read_block(priv, inode.i_block[i], block) != 0) {
            kfree(block);
            return -1; // Read error
        }
        
        uint32_t offset = 0;
        while (offset < block_size && entry_count < max_entries) {
            struct ext2_dir_entry *de = (struct ext2_dir_entry *)(block + offset);
            
            // Skip null inodes (unused entries)
            if (de->inode == 0) {
                offset += de->rec_len;
                continue;
            }
            
            // Copy entry information
            if (entries) {
                fs_dirent_t *ent = &entries[entry_count];
                ent->inode = de->inode;
                ent->name_len = de->name_len;
                ent->file_type = de->file_type;
                memcpy(ent->name, de->name, de->name_len);
                ent->name[de->name_len] = '\0';
            }
            
            entry_count++;
            offset += de->rec_len;
            
            if (de->rec_len == 0) {
                break; // Prevent infinite loop
            }
        }
    }
    
    kfree(block);
    return entry_count;
}

int ext2_get_info(ext2_private_t *priv, uint32_t inode_num, fs_file_info_t *info) {
    struct ext2_inode inode;
    
    if (!info || ext2_read_inode(priv, inode_num, &inode) != 0) {
        return -1; // Invalid parameters or failed to read inode
    }
    
    // Fill in file information
    info->size = inode.i_size;
    info->mode = inode.i_mode;
    info->uid = inode.i_uid;
    info->gid = inode.i_gid;
    info->atime = inode.i_atime;
    info->ctime = inode.i_ctime;
    info->mtime = inode.i_mtime;
    info->links = inode.i_links_count;
    
    // Set file type
    if ((inode.i_mode & EXT2_S_IFMT) == EXT2_S_IFREG) {
        info->type = FS_FILE_REGULAR;
    } else if ((inode.i_mode & EXT2_S_IFMT) == EXT2_S_IFDIR) {
        info->type = FS_FILE_DIRECTORY;
    } else if ((inode.i_mode & EXT2_S_IFMT) == EXT2_S_IFLNK) {
        info->type = FS_FILE_SYMLINK;
    } else if ((inode.i_mode & EXT2_S_IFMT) == EXT2_S_IFBLK) {
        info->type = FS_FILE_BLOCK_DEVICE;
    } else if ((inode.i_mode & EXT2_S_IFMT) == EXT2_S_IFCHR) {
        info->type = FS_FILE_CHAR_DEVICE;
    } else if ((inode.i_mode & EXT2_S_IFMT) == EXT2_S_IFIFO) {
        info->type = FS_FILE_FIFO;
    } else if ((inode.i_mode & EXT2_S_IFMT) == EXT2_S_IFSOCK) {
        info->type = FS_FILE_SOCKET;
    } else {
        info->type = FS_FILE_UNKNOWN;
    }
    
    return 0;
}

// Wrapper functions for filesystem interface
static int ext2_fs_read_file(void *private_data, const char *path, void *buf, uint32_t size, uint32_t offset) {
    ext2_private_t *priv = (ext2_private_t *)private_data;
    uint32_t inode_num;
    
    // Find the file inode
    if (ext2_find_file(priv, path, &inode_num) != 0) {
        return -1; // File not found
    }
    
    // Read the file
    uint8_t *buffer = (uint8_t *)buf;
    uint32_t remaining = size;
    uint32_t pos = 0;
    
    while (remaining > 0) {
        uint32_t to_read = (remaining > priv->block_size) ? priv->block_size : remaining;
        if (ext2_read_file(priv, inode_num, buffer + pos, to_read) != 0) {
            return -1; // Read error
        }
        pos += to_read;
        remaining -= to_read;
    }
    
    return pos;
}

static int ext2_fs_list_dir(void *private_data, const char *path, fs_dirent_t *entries, uint32_t max_entries) {
    ext2_private_t *priv = (ext2_private_t *)private_data;
    return ext2_list_dir(priv, path, entries, max_entries);
}

static int ext2_fs_get_info(void *private_data, const char *path, fs_file_info_t *info) {
    ext2_private_t *priv = (ext2_private_t *)private_data;
    uint32_t inode_num;
    
    if (strcmp(path, "/") == 0) {
        inode_num = 2; // Root directory
    } else if (ext2_find_file(priv, path, &inode_num) != 0) {
        return -1; // File not found
    }
    
    return ext2_get_info(priv, inode_num, info);
}

static int ext2_fs_find_file(void *private_data, const char *path, uint32_t *inode_out) {
    ext2_private_t *priv = (ext2_private_t *)private_data;
    return ext2_find_file(priv, path, inode_out);
}

// Filesystem operations structure
const fs_operations_t ext2_ops = {
    .read_file = ext2_fs_read_file,
    .list_dir = ext2_fs_list_dir,
    .get_info = ext2_fs_get_info,
    .find_file = ext2_fs_find_file
};

// Filesystem type structure
const filesystem_t ext2_fs = {
    .name = "ext2",
    .detect = ext2_detect,
    .mount = ext2_mount,
    .unmount = ext2_unmount,
    .ops = &ext2_ops
};