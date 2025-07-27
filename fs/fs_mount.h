#ifndef BLOODHORN_FS_MOUNT_H
#define BLOODHORN_FS_MOUNT_H
#include <stdint.h>
#include "compat.h"
int fs_mount_fat32(uint32_t lba);
int fs_mount_ext2(uint32_t lba);
int fs_mount_iso9660(uint32_t lba);
#endif 