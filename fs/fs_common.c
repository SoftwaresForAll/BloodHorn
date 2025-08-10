#include "fs_common.h"
#include "compat.h"
#include "mm.h"
#include <string.h>
#include <stddef.h>

// Global mount point list
static mount_point_t *mount_points = NULL;

// Compare two filenames (case-insensitive)
int fs_filename_cmp(const char* a, const char* b) {
    return strcasecmp(a, b);
}

// Split a path into directory and filename components
void fs_path_split(const char* path, char* dir, char* file) {
    const char* slash = strrchr(path, '/');
    if (slash) {
        size_t dir_len = slash - path;
        if (dir_len > 0) {
            strncpy(dir, path, dir_len);
            dir[dir_len] = '\0';
        } else {
            strcpy(dir, "/");
        }
        strcpy(file, slash + 1);
    } else {
        strcpy(dir, ".");
        strcpy(file, path);
    }
}

// Get the basename of a path (similar to the Unix basename command)
const char *fs_basename(const char *path) {
    const char *basename = strrchr(path, '/');
    return basename ? basename + 1 : path;
}

// Join two path components
char *fs_join_path(const char *dir, const char *file) {
    if (!dir || !file) return NULL;
    
    size_t dir_len = strlen(dir);
    size_t file_len = strlen(file);
    
    // Allocate space for the result
    char *result = (char *)kmalloc(dir_len + file_len + 2); // +2 for possible slash and null terminator
    if (!result) return NULL;
    
    // Copy directory part
    strcpy(result, dir);
    
    // Add separator if needed
    if (dir_len > 0 && dir[dir_len-1] != '/' && file[0] != '/') {
        strcat(result, "/");
    } else if (dir_len > 0 && dir[dir_len-1] == '/' && file[0] == '/') {
        // Remove duplicate slash
        file++;
    }
    
    // Append the file part
    strcat(result, file);
    
    return result;
}

// Normalize a path (remove . and .. components)
int fs_normalize_path(const char *path, char *out, size_t out_size) {
    if (!path || !out || out_size < 2) return -1;
    
    char *components[FS_MAX_PATH];
    size_t component_count = 0;
    
    // Handle absolute paths
    int is_absolute = (path[0] == '/');
    const char *current = path + (is_absolute ? 1 : 0);
    
    // Tokenize the path
    char *token;
    char *path_copy = kstrdup(path);
    if (!path_copy) return -1;
    
    token = strtok(path_copy, "/");
    while (token != NULL) {
        if (strcmp(token, ".") == 0) {
            // Skip . component
        } else if (strcmp(token, "..") == 0) {
            // Handle .. by going up one directory
            if (component_count > 0) {
                component_count--;
            } else if (!is_absolute) {
                // For relative paths, keep the .. at the beginning
                components[component_count++] = token;
            }
        } else if (token[0] != '\0') {
            // Add normal component
            if (component_count >= FS_MAX_PATH) {
                kfree(path_copy);
                return -1; // Too many components
            }
            components[component_count++] = token;
        }
        token = strtok(NULL, "/");
    }
    
    // Build the normalized path
    size_t out_pos = 0;
    
    // Start with root for absolute paths
    if (is_absolute) {
        out[out_pos++] = '/';
    }
    
    // Add all components
    for (size_t i = 0; i < component_count; i++) {
        size_t len = strlen(components[i]);
        
        // Check if we have enough space
        if (out_pos + len + (i > 0 ? 1 : 0) >= out_size) {
            kfree(path_copy);
            return -1; // Buffer too small
        }
        
        // Add separator if not the first component
        if (i > 0) {
            out[out_pos++] = '/';
        }
        
        // Copy component
        memcpy(out + out_pos, components[i], len);
        out_pos += len;
    }
    
    // Null terminate
    out[out_pos] = '\0';
    
    kfree(path_copy);
    return 0;
}

// Register a filesystem type
int fs_register(const filesystem_t *fs) {
    if (!fs) return -1;
    
    // Check if already registered
    for (mount_point_t *mp = mount_points; mp; mp = mp->next) {
        if (mp->fs == fs) {
            return 0; // Already registered
        }
    }
    
    // Create a dummy mount point for the filesystem type
    mount_point_t *new_mp = (mount_point_t *)kmalloc(sizeof(mount_point_t));
    if (!new_mp) return -1;
    
    new_mp->path = "/"; // Dummy path for registration
    new_mp->fs = fs;
    new_mp->private_data = NULL;
    new_mp->next = mount_points;
    mount_points = new_mp;
    
    return 0;
}

// Mount a filesystem at the specified path
int fs_mount(const char *path, const char *fs_type, uint32_t lba) {
    if (!path || !fs_type) return -1;
    
    // Find the filesystem type
    const filesystem_t *fs = NULL;
    for (mount_point_t *mp = mount_points; mp; mp = mp->next) {
        if (strcmp(mp->fs->name, fs_type) == 0) {
            fs = mp->fs;
            break;
        }
    }
    
    if (!fs) return -1; // Filesystem type not found
    
    // Try to mount the filesystem
    void *private_data = fs->mount(lba);
    if (!private_data) return -1; // Mount failed
    
    // Create a new mount point
    mount_point_t *new_mp = (mount_point_t *)kmalloc(sizeof(mount_point_t));
    if (!new_mp) {
        fs->unmount(private_data);
        return -1; // Out of memory
    }
    
    // Store mount point information
    new_mp->path = kstrdup(path);
    if (!new_mp->path) {
        kfree(new_mp);
        fs->unmount(private_data);
        return -1; // Out of memory
    }
    
    new_mp->fs = fs;
    new_mp->private_data = private_data;
    new_mp->next = mount_points;
    mount_points = new_mp;
    
    return 0;
}

// Unmount a filesystem
int fs_unmount(const char *path) {
    if (!path) return -1;
    
    mount_point_t **prev = &mount_points;
    mount_point_t *current = mount_points;
    
    while (current) {
        if (strcmp(current->path, path) == 0) {
            // Found the mount point
            *prev = current->next;
            
            // Unmount the filesystem
            if (current->fs && current->fs->unmount) {
                current->fs->unmount(current->private_data);
            }
            
            // Free mount point resources
            if (current->path) {
                kfree((void *)current->path);
            }
            
            kfree(current);
            return 0;
        }
        
        prev = &current->next;
        current = current->next;
    }
    
    return -1; // Mount point not found
}

// Find the mount point for a given path
mount_point_t *fs_find_mount_point(const char *path) {
    if (!path) return NULL;
    
    mount_point_t *best_match = NULL;
    size_t best_len = 0;
    
    for (mount_point_t *mp = mount_points; mp; mp = mp->next) {
        size_t mp_len = strlen(mp->path);
        
        // Check if this mount point is a prefix of the path
        if (strncmp(path, mp->path, mp_len) == 0) {
            // Check if it's a better match (longest prefix)
            if (mp_len > best_len) {
                // Also verify it's a complete path component
                if (path[mp_len] == '/' || path[mp_len] == '\0') {
                    best_match = mp;
                    best_len = mp_len;
                }
            }
        }
    }
    
    return best_match;
}

// High-level filesystem operations
int fs_read_file(const char *path, void *buf, uint32_t size, uint32_t offset) {
    if (!path || !buf) return -1;
    
    mount_point_t *mp = fs_find_mount_point(path);
    if (!mp || !mp->fs || !mp->fs->ops || !mp->fs->ops->read_file) {
        return -1; // No suitable mount point or operation not supported
    }
    
    // Skip the mount point prefix in the path
    const char *rel_path = path + strlen(mp->path);
    if (*rel_path == '/') rel_path++;
    
    return mp->fs->ops->read_file(mp->private_data, rel_path, buf, size, offset);
}

int fs_list_dir(const char *path, fs_dirent_t *entries, uint32_t max_entries) {
    if (!path) return -1;
    
    mount_point_t *mp = fs_find_mount_point(path);
    if (!mp || !mp->fs || !mp->fs->ops || !mp->fs->ops->list_dir) {
        return -1; // No suitable mount point or operation not supported
    }
    
    // Skip the mount point prefix in the path
    const char *rel_path = path + strlen(mp->path);
    if (*rel_path == '/') rel_path++;
    
    return mp->fs->ops->list_dir(mp->private_data, rel_path, entries, max_entries);
}

int fs_get_info(const char *path, fs_file_info_t *info) {
    if (!path || !info) return -1;
    
    mount_point_t *mp = fs_find_mount_point(path);
    if (!mp || !mp->fs || !mp->fs->ops || !mp->fs->ops->get_info) {
        return -1; // No suitable mount point or operation not supported
    }
    
    // Skip the mount point prefix in the path
    const char *rel_path = path + strlen(mp->path);
    if (*rel_path == '/') rel_path++;
    
    return mp->fs->ops->get_info(mp->private_data, rel_path, info);
}

int fs_find_file(const char *path, uint32_t *inode_out) {
    if (!path || !inode_out) return -1;
    
    mount_point_t *mp = fs_find_mount_point(path);
    if (!mp || !mp->fs || !mp->fs->ops || !mp->fs->ops->find_file) {
        return -1; // No suitable mount point or operation not supported
    }
    
    // Skip the mount point prefix in the path
    const char *rel_path = path + strlen(mp->path);
    if (*rel_path == '/') rel_path++;
    
    return mp->fs->ops->find_file(mp->private_data, rel_path, inode_out);
}