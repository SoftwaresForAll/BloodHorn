#ifndef BLOODHORN_EXT2_H
#define BLOODHORN_EXT2_H

#include <stdint.h>
#include <stdbool.h>
#include "compat.h"
#include "fs_mount.h"

// Constants
#define EXT2_SUPER_MAGIC      0xEF53
#define EXT2_S_IFMT           0xF000
#define EXT2_S_IFSOCK         0xC000
#define EXT2_S_IFLNK          0xA000
#define EXT2_S_IFREG          0x8000
#define EXT2_S_IFBLK          0x6000
#define EXT2_S_IFDIR          0x4000
#define EXT2_S_IFCHR          0x2000
#define EXT2_S_IFIFO          0x1000

// Inode flags
#define EXT2_SYNC_FL          0x10
#define EXT2_NOATIME_FL       0x20
#define EXT2_DIRSYNC_FL       0x40

// Superblock structure
struct ext2_superblock {
    uint32_t s_inodes_count;        // Total # of inodes
    uint32_t s_blocks_count;        // Total # of blocks
    uint32_t s_r_blocks_count;      // # of reserved blocks for superuser
    uint32_t s_free_blocks_count;   // # of free blocks
    uint32_t s_free_inodes_count;   // # of free inodes
    uint32_t s_first_data_block;    // First data block
    uint32_t s_log_block_size;      // Block size = 1024 << s_log_block_size
    uint32_t s_log_frag_size;       // Fragment size
    uint32_t s_blocks_per_group;    // # of blocks per group
    uint32_t s_frags_per_group;     // # of fragments per group
    uint32_t s_inodes_per_group;    // # of inodes per group
    uint32_t s_mtime;               // Mount time
    uint32_t s_wtime;               // Write time
    uint16_t s_mnt_count;           // Mount count
    int16_t  s_max_mnt_count;       // Max mount count
    uint16_t s_magic;               // Magic signature
    uint16_t s_state;               // File system state
    uint16_t s_errors;              // Behavior when detecting errors
    uint16_t s_minor_rev_level;     // Minor revision level
    uint32_t s_lastcheck;           // Time of last check
    uint32_t s_checkinterval;       // Max time between checks
    uint32_t s_creator_os;          // OS
    uint32_t s_rev_level;           // Revision level
    uint16_t s_def_resuid;          // Default uid for reserved blocks
    uint16_t s_def_resgid;          // Default gid for reserved blocks
    // Extended superblock fields
    uint32_t s_first_ino;           // First non-reserved inode
    uint16_t s_inode_size;          // Size of inode structure
    uint16_t s_block_group_nr;      // Block group # of this superblock
    uint32_t s_feature_compat;      // Compatible feature set
    uint32_t s_feature_incompat;    // Incompatible feature set
    uint32_t s_feature_ro_compat;   // Readonly-compatible feature set
    uint8_t  s_uuid[16];            // 128-bit uuid for volume
    char     s_volume_name[16];     // Volume name
    char     s_last_mounted[64];    // Directory where mounted
    uint32_t s_algorithm_usage_bitmap; // Used for compression
    // Performance hints
    uint8_t  s_prealloc_blocks;     // # of blocks to try to preallocate
    uint8_t  s_prealloc_dir_blocks; // # of blocks to preallocate for directories
    uint16_t s_padding1;
    // Journaling support
    uint8_t  s_journal_uuid[16];    // UUID of journal superblock
    uint32_t s_journal_inum;        // Inode number of journal file
    uint32_t s_journal_dev;         // Device number of journal file
    uint32_t s_last_orphan;         // Start of list of orphaned inodes
    uint32_t s_hash_seed[4];        // HTREE hash seed
    uint8_t  s_def_hash_version;    // Default hash version to use
    uint8_t  s_jnl_backup_type;     // Type of backup
    uint16_t s_reserved_word_pad;
    uint32_t s_default_mount_opts;
    uint32_t s_first_meta_bg;       // First metablock group
    uint32_t s_mkfs_time;           // When the filesystem was created
    uint32_t s_jnl_blocks[17];      // Backup of the journal inode
    // 64-bit support
    uint32_t s_blocks_count_hi;     // High 32 bits of block count
    uint32_t s_r_blocks_count_hi;   // High 32 bits of reserved block count
    uint32_t s_free_blocks_hi;      // High 32 bits of free block count
    uint16_t s_min_extra_isize;     // All inodes have at least this size
    uint16_t s_want_extra_isize;    // New inodes should reserve this much space
    uint32_t s_flags;               // Miscellaneous flags
    uint16_t s_raid_stride;         // RAID stride
    uint16_t s_mmp_interval;        // # seconds to wait in multi-mount protection
    uint64_t s_mmp_block;           // Block # for multi-mount protection
    uint32_t s_raid_stripe_width;   // Blocks on all data disks (N * stride)
    uint8_t  s_log_groups_per_flex; // FLEX_BG group size
    uint8_t  s_checksum_type;       // Checksum algorithm used
    uint8_t  s_encryption_level;    // Versioning level for encryption
    uint8_t  s_reserved_pad;        // Padding to next 32-bit boundary
    uint64_t s_kbytes_written;      // # of KiB written to this filesystem
    uint32_t s_snapshot_inum;       // Inode number of active snapshot
    uint32_t s_snapshot_id;         // Sequential ID of snapshot
    uint64_t s_snapshot_r_blocks_count; // # of blocks reserved for snapshot
    uint32_t s_snapshot_list;       // Inode number of the head of the snapshot list
    uint32_t s_error_count;         // Number of errors seen
    uint32_t s_first_error_time;    // First time an error happened
    uint32_t s_first_error_ino;     // Inode involved in first error
    uint64_t s_first_error_block;   // Block involved in first error
    uint8_t  s_first_error_func[32];// Function where the error happened
    uint32_t s_first_error_line;    // Line number where error happened
    uint32_t s_last_error_time;     // Most recent time of an error
    uint32_t s_last_error_ino;      // Inode involved in last error
    uint32_t s_last_error_line;     // Line number where last error happened
    uint64_t s_last_error_block;    // Block involved in last error
    uint8_t  s_last_error_func[32]; // Function where last error happened
    uint8_t  s_mount_opts[64];      // Mount options
    uint32_t s_usr_quota_inum;      // Inode for tracking user quota
    uint32_t s_grp_quota_inum;      // Inode for tracking group quota
    uint32_t s_overhead_blocks;     // Overhead blocks/clusters in fs
    uint32_t s_backup_bgs[2];       // Groups with sparse_super2 SBs
    uint8_t  s_encrypt_algos[4];    // Encryption algorithms in use
    uint8_t  s_encrypt_pw_salt[16]; // Salt used for string2key algorithm
    uint32_t s_lpf_ino;             // Location of the lost+found inode
    uint32_t s_prj_quota_inum;      // Inode for tracking project quota
    uint32_t s_checksum_seed;       // Checksum seed
    uint32_t s_reserved[98];        // Padding to the end of the block
    uint32_t s_checksum;            // crc32c(superblock)
} __attribute__((packed));

// Block group descriptor
struct ext2_group_desc {
    uint32_t bg_block_bitmap;       // Block address of block bitmap
    uint32_t bg_inode_bitmap;       // Block address of inode bitmap
    uint32_t bg_inode_table;        // Block address of inode table
    uint16_t bg_free_blocks_count;  // Number of free blocks
    uint16_t bg_free_inodes_count;  // Number of free inodes
    uint16_t bg_used_dirs_count;    // Number of directories
    uint16_t bg_pad;                // Padding
    uint32_t bg_reserved[3];        // Reserved for future use
} __attribute__((packed));

// Inode structure
struct ext2_inode {
    uint16_t i_mode;                // File mode
    uint16_t i_uid;                 // Owner UID
    uint32_t i_size;                // Size in bytes
    uint32_t i_atime;               // Access time
    uint32_t i_ctime;               // Creation time
    uint32_t i_mtime;              // Modification time
    uint32_t i_dtime;              // Deletion time
    uint16_t i_gid;                // Group ID
    uint16_t i_links_count;        // Links count
    uint32_t i_blocks;             // Blocks count
    uint32_t i_flags;              // File flags
    uint32_t i_osd1;               // OS dependent 1
    uint32_t i_block[15];          // Pointers to blocks
    uint32_t i_generation;         // File version (for NFS)
    uint32_t i_file_acl;           // File ACL
    uint32_t i_size_high;          // High 32 bits of file size
    uint32_t i_faddr;              // Fragment address
    uint8_t  i_osd2[12];           // OS dependent 2
} __attribute__((packed));

// Directory entry structure
struct ext2_dir_entry {
    uint32_t inode;         // Inode number
    uint16_t rec_len;       // Directory entry length
    uint8_t  name_len;      // Name length
    uint8_t  file_type;     // File type
    char     name[255];     // File name (up to 255 bytes)
} __attribute__((packed));

// File types
#define EXT2_FT_UNKNOWN     0
#define EXT2_FT_REG_FILE    1
#define EXT2_FT_DIR         2
#define EXT2_FT_CHRDEV      3
#define EXT2_FT_BLKDEV      4
#define EXT2_FT_FIFO        5
#define EXT2_FT_SOCK        6
#define EXT2_FT_SYMLINK     7

// ext2 private data structure
typedef struct {
    uint32_t lba;                       // Starting LBA of partition
    struct ext2_superblock sb;          // Superblock
    uint32_t block_size;                // Block size in bytes
    uint32_t inode_size;                // Inode size in bytes
    uint32_t blocks_per_group;          // Blocks per group
    uint32_t inodes_per_group;          // Inodes per group
    uint32_t inodes_per_block;          // Inodes per block
    uint32_t desc_per_block;            // Descriptors per block
    uint32_t group_count;               // Total number of block groups
    struct ext2_group_desc *gd;         // Block group descriptors
} ext2_private_t;

// ext2 filesystem operations
extern const fs_operations_t ext2_ops;

// ext2 filesystem type
extern const filesystem_t ext2_fs;

// Helper functions
int ext2_read_inode(ext2_private_t *priv, uint32_t inode_num, struct ext2_inode *inode);
int ext2_read_block(ext2_private_t *priv, uint32_t block_num, void *buffer);
int ext2_find_file(ext2_private_t *priv, const char *path, uint32_t *inode_num, uint32_t *size);

#endif // BLOODHORN_EXT2_H
