#include "fs_mount.h"
#include "compat.h"
#include "fat32.h"
#include "ext2.h"
#include "iso9660.h"
#include <string.h>
#include <stdlib.h>

// List of registered filesystems
static const filesystem_t *registered_fs[8] = {0};
static int num_registered_fs = 0;

// List of mount points
static mount_point_t *mount_points = NULL;

// Helper function to find a filesystem by name
static const filesystem_t *find_filesystem(const char *name) {
    for (int i = 0; i < num_registered_fs; i++) {
        if (strcmp(registered_fs[i]->name, name) == 0) {
            return registered_fs[i];
        }
    }
    return NULL;
}

// Helper function to find mount point for a path
static mount_point_t *find_mount_point(const char *path) {
    mount_point_t *best_match = NULL;
    size_t best_len = 0;
    size_t path_len = strlen(path);
    
    for (mount_point_t *mp = mount_points; mp != NULL; mp = mp->next) {
        size_t mp_len = strlen(mp->path);
        if (path_len >= mp_len && 
            strncmp(path, mp->path, mp_len) == 0 &&
            (path[mp_len] == '/' || path[mp_len] == '\' || path[mp_len] == '\0') &&
            mp_len > best_len) {
            best_match = mp;
            best_len = mp_len;
        }
    }
    
    return best_match;
}

// Register a filesystem type
void fs_register(const filesystem_t *fs) {
    if (num_registered_fs < (int)(sizeof(registered_fs)/sizeof(registered_fs[0]))) {
        registered_fs[num_registered_fs++] = fs;
    }
}

// Mount a filesystem
int fs_mount(const char *path, const char *fstype, uint32_t lba, void *opts) {
    // Find the filesystem
    const filesystem_t *fs = find_filesystem(fstype);
    if (!fs) {
        return -1; // Filesystem type not found
    }
    
    // Allocate and initialize mount point
    mount_point_t *mp = (mount_point_t *)malloc(sizeof(mount_point_t));
    if (!mp) {
        return -2; // Out of memory
    }
    
    mp->path = strdup(path);
    mp->fs = fs;
    mp->start_lba = lba;
    mp->next = NULL;
    
    // Call filesystem-specific mount function
    mp->private_data = fs->mount(lba, opts);
    if (!mp->private_data) {
        free((void *)mp->path);
        free(mp);
        return -3; // Mount failed
    }
    
    // Add to mount points list
    if (!mount_points) {
        mount_points = mp;
    } else {
        mount_point_t *last = mount_points;
        while (last->next) last = last->next;
        last->next = mp;
    }
    
    return 0;
}

// Unmount a filesystem
int fs_unmount(const char *path) {
    mount_point_t **pmp = &mount_points;
    while (*pmp) {
        if (strcmp((*pmp)->path, path) == 0) {
            mount_point_t *mp = *pmp;
            *pmp = mp->next;
            
            // Call filesystem-specific unmount
            if (mp->fs->unmount) {
                mp->fs->unmount(mp->private_data);
            }
            
            free((void *)mp->path);
            free(mp);
            return 0;
        }
        pmp = &(*pmp)->next;
    }
    return -1; // Not found
}

// Get mount point for a path
mount_point_t *fs_get_mount_point(const char *path) {
    return find_mount_point(path);
}

// File operations
int fs_read(const char *path, uint8_t *buf, uint32_t size, uint32_t offset) {
    mount_point_t *mp = find_mount_point(path);
    if (!mp || !mp->fs->ops->read) {
        return -1;
    }
    
    // Skip mount point path
    const char *rel_path = path + strlen(mp->path);
    if (*rel_path == '/' || *rel_path == '\\') {
        rel_path++;
    }
    
    return mp->fs->ops->read(mp, rel_path, buf, size, offset);
}

int fs_write(const char *path, const uint8_t *buf, uint32_t size, uint32_t offset) {
    mount_point_t *mp = find_mount_point(path);
    if (!mp || !mp->fs->ops->write) {
        return -1;
    }
    
    const char *rel_path = path + strlen(mp->path);
    if (*rel_path == '/' || *rel_path == '\\') {
        rel_path++;
    }
    
    return mp->fs->ops->write(mp, rel_path, buf, size, offset);
}

int fs_list_dir(const char *path, char *buffer, uint32_t size) {
    mount_point_t *mp = find_mount_point(path);
    if (!mp || !mp->fs->ops->list_dir) {
        return -1;
    }
    
    const char *rel_path = path + strlen(mp->path);
    if (*rel_path == '/' || *rel_path == '\\') {
        rel_path++;
    }
    
    return mp->fs->ops->list_dir(mp, rel_path, buffer, size);
}

int fs_get_info(const char *path, uint32_t *size, bool *is_dir) {
    mount_point_t *mp = find_mount_point(path);
    if (!mp || !mp->fs->ops->get_info) {
        return -1;
    }
    
    const char *rel_path = path + strlen(mp->path);
    if (*rel_path == '/' || *rel_path == '\\') {
        rel_path++;
    }
    
    return mp->fs->ops->get_info(mp, rel_path, size, is_dir);
}

// Path helper functions
const char *fs_basename(const char *path) {
    const char *base = path;
    while (*path) {
        if (*path == '/' || *path == '\\') {
            base = path + 1;
        }
        path++;
    }
    return base;
}

char *fs_dirname(const char *path, char *buf, size_t size) {
    if (size == 0) return NULL;
    
    const char *end = path + strlen(path) - 1;
    while (end > path && (*end == '/' || *end == '\\')) {
        end--;
    }
    
    while (end > path && *end != '/' && *end != '\\') {
        end--;
    }
    
    size_t len = (end - path) + 1;
    if (len >= size) {
        len = size - 1;
    }
    
    strncpy(buf, path, len);
    buf[len] = '\0';
    return buf;
}

char *fs_join_path(const char *dir, const char *file, char *buf, size_t size) {
    if (size == 0) return NULL;
    
    size_t dir_len = strlen(dir);
    size_t file_len = strlen(file);
    
    // Remove trailing slashes from dir
    while (dir_len > 0 && (dir[dir_len-1] == '/' || dir[dir_len-1] == '\\')) {
        dir_len--;
    }
    
    // Remove leading slashes from file
    while (*file == '/' || *file == '\\') {
        file++;
        file_len--;
    }
    
    size_t total_len = dir_len + 1 + file_len + 1;
    if (total_len > size) {
        return NULL; // Buffer too small
    }
    
    char *p = buf;
    memcpy(p, dir, dir_len);
    p += dir_len;
    *p++ = '/';
    memcpy(p, file, file_len);
    p[file_len] = '\0';
    
    return buf;
}

// Initialize filesystem layer
void fs_init(void) {
    // Register built-in filesystems
    fs_register(&fat32_fs);
    fs_register(&ext2_fs);
    fs_register(&iso9660_fs);
    
    // Auto-detect and mount root filesystem
    // This is just an example - in a real implementation, you'd want to
    // scan available disks and partitions to find the root filesystem
    uint32_t root_lba = 0; // This would be determined by partition scanning
    
    // Try to detect and mount the root filesystem
    for (int i = 0; i < num_registered_fs; i++) {
        if (registered_fs[i]->detect && registered_fs[i]->detect(root_lba)) {
            if (fs_mount("/", registered_fs[i]->name, root_lba, NULL) == 0) {
                break; // Successfully mounted root
            }
        }
    }
}