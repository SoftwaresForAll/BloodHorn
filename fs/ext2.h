#ifndef BLOODHORN_EXT2_H
#define BLOODHORN_EXT2_H
#include <stdint.h>
#include "compat.h"

struct ext2_super_block {
    uint32_t   s_inodes_count;
    uint32_t   s_blocks_count;
    uint32_t   s_r_blocks_count;
    uint32_t   s_free_blocks_count;
    uint32_t   s_free_inodes_count;
    uint32_t   s_first_data_block;
    uint32_t   s_log_block_size;
    uint32_t   s_log_frag_size;
    uint32_t   s_blocks_per_group;
    uint32_t   s_frags_per_group;
    uint32_t   s_inodes_per_group;
    uint32_t   s_mtime;
    uint32_t   s_wtime;
    uint16_t   s_mnt_count;
    uint16_t   s_max_mnt_count;
    uint16_t   s_magic;
    // ... (add more fields as needed)
};

#endif // BLOODHORN_EXT2_H 