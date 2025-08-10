#ifndef BLOODHORN_FS_MOUNT_H
#define BLOODHORN_FS_MOUNT_H

#include <stdint.h>
#include <stdbool.h>
#include "compat.h"

// Forward declarations
typeof struct fs_operations fs_operations_t;
typeof struct filesystem filesystem_t;
typeof struct mount_point mount_point_t;

// File operations structure
typedef struct fs_operations {
    int (*read)(mount_point_t *mp, const char *path, uint8_t *buf, uint32_t size, uint32_t offset);
    int (*write)(mount_point_t *mp, const char *path, const uint8_t *buf, uint32_t size, uint32_t offset);
    int (*list_dir)(mount_point_t *mp, const char *path, char *buffer, uint32_t size);
    int (*get_info)(mount_point_t *mp, const char *path, uint32_t *size, bool *is_dir);
} fs_operations_t;

// Filesystem type structure
typedef struct filesystem {
    const char *name;
    const fs_operations_t *ops;
    bool (*detect)(uint32_t lba);
    void* (*mount)(uint32_t lba, void *opts);
    void (*unmount)(void *private_data);
} filesystem_t;

// Mount point structure
typedef struct mount_point {
    const char *path;           // Mount path (e.g., "/", "/boot")
    const filesystem_t *fs;     // Filesystem type
    void *private_data;         // Filesystem-specific data
    uint32_t start_lba;         // Starting LBA of the partition
    struct mount_point *next;   // Next mount point in the list
} mount_point_t;

// Filesystem registration
void fs_register(const filesystem_t *fs);

// Mount management
int fs_mount(const char *path, const char *fstype, uint32_t lba, void *opts);
int fs_unmount(const char *path);
mount_point_t *fs_get_mount_point(const char *path);

// File operations
int fs_read(const char *path, uint8_t *buf, uint32_t size, uint32_t offset);
int fs_write(const char *path, const uint8_t *buf, uint32_t size, uint32_t offset);
int fs_list_dir(const char *path, char *buffer, uint32_t size);
int fs_get_info(const char *path, uint32_t *size, bool *is_dir);

// Helper functions
const char *fs_basename(const char *path);
char *fs_dirname(const char *path, char *buf, size_t size);
char *fs_join_path(const char *dir, const char *file, char *buf, size_t size);

// Initialize filesystem layer
void fs_init(void);

// Built-in filesystem types
extern const filesystem_t fat32_fs;
extern const filesystem_t ext2_fs;
extern const filesystem_t iso9660_fs;

#endif // BLOODHORN_FS_MOUNT_H