#ifndef BLOODHORN_ISO9660_H
#define BLOODHORN_ISO9660_H

#include <stdint.h>
#include "compat.h"
#include "fs_common.h"

// ISO9660 Primary Volume Descriptor
struct iso_volume_descriptor {
    uint8_t type;                   // 0x01 for primary volume descriptor
    char standard_id[5];            // "CD001"
    uint8_t version;                // 0x01
    uint8_t unused1;
    char system_id[32];             // System identifier
    char volume_id[32];             // Volume identifier
    uint8_t unused2[8];
    uint32_t volume_space_size;     // Number of logical blocks in volume
    uint8_t unused3[32];
    uint16_t volume_set_size;       // Volume set size
    uint16_t volume_sequence_number; // Volume sequence number
    uint16_t logical_block_size;    // Logical block size (usually 2048 or 4096)
    uint32_t path_table_size;       // Path table size in bytes
    uint32_t path_table_l;          // LBA of first occurrence of path table
    uint32_t path_table_opt_l;      // LBA of optional path table
    uint32_t path_table_m;          // LBA of path table (big-endian)
    uint32_t path_table_opt_m;      // LBA of optional path table (big-endian)
    uint8_t root_directory_record[34]; // Root directory record
    char volume_set_id[128];        // Volume set identifier
    char publisher_id[128];         // Publisher identifier
    char data_preparer_id[128];     // Data preparer identifier
    char application_id[128];       // Application identifier
    char copyright_file_id[37];     // Copyright file identifier
    char abstract_file_id[37];      // Abstract file identifier
    char bibliographic_file_id[37]; // Bibliographic file identifier
    uint8_t creation_date[17];      // Creation date (YYYYMMDDHHMMSSsss)
    uint8_t modification_date[17];  // Modification date
    uint8_t expiration_date[17];    // Expiration date
    uint8_t effective_date[17];     // Effective date
    uint8_t file_structure_version; // File structure version (0x01)
    uint8_t unused4;
    uint8_t application_use[512];   // Application use
    uint8_t reserved[653];          // Reserved by ISO
} __attribute__((packed));

// ISO9660 Directory Record
struct iso_directory_record {
    uint8_t length;                 // Length of directory record
    uint8_t ext_attr_length;        // Extended attribute length
    uint32_t extent_l;              // Extent location (LBA)
    uint32_t extent_m;              // Extent location (big-endian)
    uint32_t data_length_l;         // Data length in bytes
    uint32_t data_length_m;         // Data length (big-endian)
    uint8_t date[7];                // Recording date and time
    uint8_t file_flags;             // File flags
    uint8_t file_unit_size;         // File unit size for interleaved files
    uint8_t interleave_gap;         // Interleave gap size
    uint16_t volume_sequence_number_l; // Volume sequence number
    uint16_t volume_sequence_number_m; // Volume sequence number (big-endian)
    uint8_t name_len;               // Length of file identifier
    char name[];                    // File identifier (variable length, not null-terminated)
} __attribute__((packed));

// ISO9660 Path Table Entry
struct iso_path_table_entry {
    uint8_t name_len;               // Length of directory name
    uint8_t ext_attr_length;        // Extended attribute length
    uint32_t extent;                // Extent location (LBA)
    uint16_t parent_dir_num;        // Parent directory number
    char name[];                    // Directory name (variable length, not null-terminated)
} __attribute__((packed));

// ISO9660 private data structure
typedef struct {
    uint32_t lba;                   // Starting LBA of the ISO9660 volume
    uint32_t block_size;            // Logical block size (usually 2048)
    struct iso_volume_descriptor pvd; // Primary Volume Descriptor
    uint8_t *path_table;            // Path table buffer
    uint32_t path_table_size;       // Size of path table in bytes
} iso9660_private_t;

// ISO9660 filesystem operations
extern const fs_operations_t iso9660_ops;

// ISO9660 filesystem type
extern const filesystem_t iso9660_fs;

// Helper functions
int iso9660_read_inode(iso9660_private_t *priv, uint32_t inode_num, struct iso_directory_record *record);
int iso9660_read_block(iso9660_private_t *priv, uint32_t block_num, void *buffer);
int iso9660_find_file(iso9660_private_t *priv, const char *path, uint32_t *extent, uint32_t *size);

#endif // BLOODHORN_ISO9660_H