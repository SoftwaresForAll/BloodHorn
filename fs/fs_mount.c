#include "fs_mount.h"
#include "compat.h"
#include "fat32.h"
#include "ext2.h"
#include "iso9660.h"
int fs_mount_fat32(uint32_t lba) { return fat32_read_bootsector(lba, 0) == 0; }
int fs_mount_ext2(uint32_t lba) { return ext2_read_superblock(lba, 0) == 0; }
int fs_mount_iso9660(uint32_t lba) { return iso9660_read_file(lba, "README.TXT", 0, 0) == 0; } 