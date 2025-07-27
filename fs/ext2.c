#include <stdint.h>
#include "compat.h"
#include <stddef.h>
#include <string.h>
#include "ext2.h"

extern void read_sector(uint32_t lba, uint8_t* buf);

static void ext2_read_block(uint32_t lba, uint32_t block, uint32_t block_size, uint8_t* buf) {
    for (uint32_t i = 0; i < block_size / 512; ++i)
        read_sector(lba + block * (block_size / 512) + i, buf + i * 512);
}

int ext2_read_superblock(uint32_t lba, struct ext2_super_block* sb) {
    uint8_t buf[1024];
    read_sector(lba + 2, buf);
    memcpy(sb, buf, sizeof(struct ext2_super_block));
    return 0;
}

static void ext2_read_inode(uint32_t lba, const struct ext2_super_block* sb, uint32_t inode_num, uint8_t* inode) {
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint8_t group_desc[32];
    read_sector(lba + 2 + (block_size / 512), group_desc);
    uint32_t inode_table = *(uint32_t*)&group_desc[8];
    uint32_t inode_size = sb->s_inode_size;
    uint32_t inodes_per_block = block_size / inode_size;
    uint32_t block = inode_table + (inode_num - 1) / inodes_per_block;
    uint8_t block_buf[4096];
    ext2_read_block(lba, block, block_size, block_buf);
    memcpy(inode, block_buf + ((inode_num - 1) % inodes_per_block) * inode_size, inode_size);
}

int ext2_find_file_in_root(uint32_t lba, const char* filename, uint32_t* inode_out) {
    struct ext2_super_block sb;
    ext2_read_superblock(lba, &sb);
    uint32_t block_size = 1024 << sb.s_log_block_size;
    uint8_t inode[256];
    ext2_read_inode(lba, &sb, 2, inode); // root inode
    uint32_t* block_ptrs = (uint32_t*)(inode + 40);
    uint8_t dir[4096];
    ext2_read_block(lba, block_ptrs[0], block_size, dir);
    uint32_t offset = 0;
    while (offset < block_size) {
        uint32_t inode_num = *(uint32_t*)&dir[offset];
        uint16_t rec_len = *(uint16_t*)&dir[offset+4];
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

int ext2_read_file(uint32_t lba, uint32_t inode_num, uint8_t* buf, uint32_t max_size) {
    struct ext2_super_block sb;
    ext2_read_superblock(lba, &sb);
    uint32_t block_size = 1024 << sb.s_log_block_size;
    uint8_t inode[256];
    ext2_read_inode(lba, &sb, inode_num, inode);
    uint32_t* block_ptrs = (uint32_t*)(inode + 40);
    uint32_t file_size = *(uint32_t*)(inode + 4);
    uint32_t to_read = (file_size < max_size) ? file_size : max_size;
    uint32_t read_bytes = 0;
    for (int i = 0; i < 12 && read_bytes < to_read; ++i) {
        if (block_ptrs[i] == 0) break;
        uint32_t chunk = (to_read - read_bytes > block_size) ? block_size : (to_read - read_bytes);
        ext2_read_block(lba, block_ptrs[i], block_size, buf + read_bytes);
        read_bytes += chunk;
    }
    return 0;
} 