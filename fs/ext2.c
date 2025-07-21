#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "ext2.h"

extern void read_sector(uint32_t lba, uint8_t* buf);

int ext2_read_superblock(uint32_t lba, struct ext2_super_block* sb) {
    uint8_t buf[512];
    read_sector(lba + 2, buf);
    for (size_t i = 0; i < sizeof(struct ext2_super_block); ++i)
        ((uint8_t*)sb)[i] = buf[i];
    return 0;
}

int ext2_find_file_in_root(uint32_t lba, const char* filename, uint32_t* inode_out) {
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
    uint32_t offset = 0;
    while (offset < 1024) {
        uint32_t inode_num = *(uint32_t*)&dir[offset];
        uint16_t rec_len = *(uint16_t*)&dir[offset+4];
        uint8_t name_len = dir[offset+6];
        char name[256];
        for (int i = 0; i < name_len; ++i) name[i] = dir[offset+8+i];
        name[name_len] = 0;
        if (inode_num && strcmp(name, filename) == 0) {
            *inode_out = inode_num;
            return 0;
        }
        offset += rec_len;
        if (rec_len == 0) break;
    }
    return -1;
}

int ext2_read_file(uint32_t lba, uint32_t inode_num, uint8_t* buf, uint32_t max_size) {
    struct ext2_super_block sb;
    ext2_read_superblock(lba, &sb);
    uint32_t block_size = 1024 << sb.s_log_block_size;
    uint8_t group_desc[32];
    read_sector(lba + 2 + (block_size / 512), group_desc);
    uint32_t inode_table = *(uint32_t*)&group_desc[8];
    uint8_t inode[128];
    read_sector(lba + inode_table * (block_size / 512) + (inode_num-1), inode);
    uint32_t file_block = *(uint32_t*)&inode[40];
    read_sector(lba + file_block * (block_size / 512), buf);
    return 0;
} 