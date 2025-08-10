#ifndef BLOODHORN_FS_COMMON_H
#define BLOODHORN_FS_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include "compat.h"

// Maximum length for a filename
#define FS_MAX_FILENAME 256
#define FS_MAX_PATH 4096

// File types
enum {
    FS_FILE_UNKNOWN = 0,
    FS_FILE_REGULAR,
    FS_FILE_DIRECTORY,
    FS_FILE_SYMLINK,
    FS_FILE_BLOCK_DEVICE,
    FS_FILE_CHAR_DEVICE,
    FS_FILE_FIFO,
    FS_FILE_SOCKET
};

// Directory entry structure
typedef struct {
    uint32_t inode;         // Inode number
    char name[FS_MAX_FILENAME]; // File name (null-terminated)
    uint8_t name_len;       // Length of the name
    uint8_t file_type;      // File type (from enum above)
} fs_dirent_t;

// File information structure
typedef struct {
    uint64_t size;          // File size in bytes
    uint32_t mode;          // File mode and permissions
    uint32_t uid;           // User ID of owner
    uint32_t gid;           // Group ID of owner
    uint64_t atime;         // Last access time
    uint64_t ctime;         // Creation time
    uint64_t mtime;         // Last modification time
    uint32_t links;         // Number of hard links
    uint8_t type;           // File type (from enum above)
} fs_file_info_t;

// Filesystem operations structure
typedef struct fs_operations {
    // Read file contents into buffer
    int (*read_file)(void *private_data, const char *path, void *buf, uint32_t size, uint32_t offset);
    
    // List directory contents
    int (*list_dir)(void *private_data, const char *path, fs_dirent_t *entries, uint32_t max_entries);
    
    // Get file/directory information
    int (*get_info)(void *private_data, const char *path, fs_file_info_t *info);
    
    // Find a file and return its inode
    int (*find_file)(void *private_data, const char *path, uint32_t *inode_out);
} fs_operations_t;

// Filesystem type structure
typedef struct filesystem {
    const char *name;                       // Filesystem name (e.g., "fat32", "ext2")
    int (*detect)(uint32_t lba);           // Detect if this filesystem is present
    void* (*mount)(uint32_t lba);          // Mount the filesystem
    void (*unmount)(void *private_data);   // Unmount the filesystem
    const struct fs_operations *ops;       // Filesystem operations
} filesystem_t;

// Mount point structure
typedef struct mount_point {
    const char *path;           // Mount path (e.g., "/", "/boot")
    const filesystem_t *fs;     // Filesystem type
    void *private_data;         // Filesystem private data
    struct mount_point *next;   // Next mount point in the list
} mount_point_t;

// Function declarations
int fs_filename_cmp(const char* a, const char* b);
void fs_path_split(const char* path, char* dir, char* file);
const char *fs_basename(const char *path);
char *fs_join_path(const char *dir, const char *file);
int fs_normalize_path(const char *path, char *out, size_t out_size);

// Filesystem registration and mounting
int fs_register(const filesystem_t *fs);
int fs_mount(const char *path, const char *fs_type, uint32_t lba);
int fs_unmount(const char *path);
mount_point_t *fs_find_mount_point(const char *path);

// High-level filesystem operations
int fs_read_file(const char *path, void *buf, uint32_t size, uint32_t offset);
int fs_list_dir(const char *path, fs_dirent_t *entries, uint32_t max_entries);
int fs_get_info(const char *path, fs_file_info_t *info);
int fs_find_file(const char *path, uint32_t *inode_out);

#endif // BLOODHORN_FS_COMMON_H