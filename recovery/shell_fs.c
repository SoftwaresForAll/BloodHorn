#include "shell_fs.h"
#include "compat.h"
#include "../fs/fat32.h"
#include "../fs/ext2.h"
#include <string.h>
#include <stdio.h>

int shell_cmd_ls_fat32(uint32_t lba, char* out, int maxlen) {
    struct fat32_bootsector bs;
    fat32_read_bootsector(lba, &bs);
    uint32_t fat_offset = bs.reserved_sectors;
    uint32_t root_dir_sectors = ((bs.root_entries * 32) + (bs.bytes_per_sector - 1)) / bs.bytes_per_sector;
    uint32_t root_dir_start = lba + fat_offset + (bs.num_fats * bs.sectors_per_fat);
    uint8_t dir[512];
    read_sector(root_dir_start, dir);
    int pos = 0;
    for (int i = 0; i < 512; i += 32) {
        if (dir[i] == 0) break;
        if (dir[i] == 0xE5) continue;
        if ((dir[i+11] & 0x10) != 0) continue;
        char name[12];
        memcpy(name, &dir[i], 11);
        name[11] = 0;
        int n = snprintf(out + pos, maxlen - pos, "%s ", name);
        pos += n;
        if (pos >= maxlen - 1) break;
    }
    out[pos] = 0;
    return 0;
}

int shell_cmd_cat_fat32(uint32_t lba, const char* filename, char* out, int maxlen) {
    uint32_t cluster;
    if (fat32_find_file(lba, filename, &cluster) != 0) return -1;
    fat32_read_file(lba, cluster, (uint8_t*)out, maxlen);
    out[maxlen-1] = 0;
    return 0;
}

int shell_cmd_ls_ext2(uint32_t lba, char* out, int maxlen) {
    struct ext2_super_block sb;
    ext2_read_superblock(lba, &sb);
    uint32_t block_size = 1024 << sb.s_log_block_size;
    uint8_t group_desc[32];
    read_sector(lba + 2 + (block_size / 512), group_desc);
    uint32_t inode_table = *(uint32_t*)&group_desc[8];
    uint8_t inode[128];
    read_sector(lba + inode_table * (block_size / 512), inode);
    uint32_t dir_block = *(uint32_t*)&inode[40];
    uint8_t dir[1024];
    read_sector(lba + dir_block * (block_size / 512), dir);
    int pos = 0;
    uint32_t offset = 0;
    while (offset < 1024) {
        uint32_t inode_num = *(uint32_t*)&dir[offset];
        uint16_t rec_len = *(uint16_t*)&dir[offset+4];
        uint8_t name_len = dir[offset+6];
        char name[256];
        for (int i = 0; i < name_len; ++i) name[i] = dir[offset+8+i];
        name[name_len] = 0;
        if (inode_num) {
            int n = snprintf(out + pos, maxlen - pos, "%s ", name);
            pos += n;
            if (pos >= maxlen - 1) break;
        }
        offset += rec_len;
        if (rec_len == 0) break;
    }
    out[pos] = 0;
    return 0;
}

int shell_cmd_cat_ext2(uint32_t lba, const char* filename, char* out, int maxlen) {
    uint32_t inode_num;
    if (ext2_find_file_in_root(lba, filename, &inode_num) != 0) return -1;
    ext2_read_file(lba, inode_num, (uint8_t*)out, maxlen);
    out[maxlen-1] = 0;
    return 0;
} 