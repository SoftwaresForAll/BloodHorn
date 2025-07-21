#ifndef BLOODHORN_CHAINLOAD_H
#define BLOODHORN_CHAINLOAD_H
#include <stdint.h>

struct mbr_partition {
    uint8_t status;
    uint8_t chs_start[3];
    uint8_t type;
    uint8_t chs_end[3];
    uint32_t lba_start;
    uint32_t sector_count;
};

struct mbr {
    uint8_t bootstrap[446];
    struct mbr_partition partitions[4];
    uint16_t signature;
};

struct gpt_header {
    uint64_t signature;
    uint32_t revision;
    uint32_t header_size;
    uint32_t header_crc32;
    uint32_t reserved;
    uint64_t current_lba;
    uint64_t backup_lba;
    uint64_t first_usable_lba;
    uint64_t last_usable_lba;
    uint8_t disk_guid[16];
    uint64_t partition_entry_lba;
    uint32_t num_partition_entries;
    uint32_t size_of_partition_entry;
    uint32_t partition_entry_array_crc32;
};

struct gpt_partition {
    uint8_t partition_type_guid[16];
    uint8_t unique_partition_guid[16];
    uint64_t starting_lba;
    uint64_t ending_lba;
    uint64_t attributes;
    uint16_t partition_name[36];
};

// MBR partition types
#define PARTITION_TYPE_EMPTY 0x00
#define PARTITION_TYPE_FAT12 0x01
#define PARTITION_TYPE_FAT16 0x04
#define PARTITION_TYPE_EXTENDED 0x05
#define PARTITION_TYPE_FAT16B 0x06
#define PARTITION_TYPE_NTFS 0x07
#define PARTITION_TYPE_FAT32 0x0B
#define PARTITION_TYPE_FAT32_LBA 0x0C
#define PARTITION_TYPE_EXTENDED_LBA 0x0F
#define PARTITION_TYPE_LINUX_SWAP 0x82
#define PARTITION_TYPE_LINUX_NATIVE 0x83
#define PARTITION_TYPE_LINUX_EXTENDED 0x85
#define PARTITION_TYPE_LINUX_LVM 0x8E
#define PARTITION_TYPE_EFI_SYSTEM 0xEF

// GPT partition type GUIDs
#define GPT_PARTITION_TYPE_EFI_SYSTEM "C12A7328-F81F-11D2-BA4B-00A0C93EC93B"
#define GPT_PARTITION_TYPE_MICROSOFT_BASIC "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7"
#define GPT_PARTITION_TYPE_MICROSOFT_LDM "5808C8AA-7E8F-42E0-85D2-E1E90434CFB3"
#define GPT_PARTITION_TYPE_MICROSOFT_LDM_METADATA "A9A0A0A2-B9E5-4433-87C0-68B6B72699C7"
#define GPT_PARTITION_TYPE_LINUX_FILESYSTEM "0FC63DAF-8483-4772-8E79-3D69D8477DE4"
#define GPT_PARTITION_TYPE_LINUX_SWAP "0657FD6D-A4AB-43C4-84E5-0933C84B4F4F"
#define GPT_PARTITION_TYPE_LINUX_LVM "E6D6D379-F507-44C2-A23C-238F2A3DF928"
#define GPT_PARTITION_TYPE_LINUX_RAID "A19D880F-05FC-4D3B-A006-743F0F84911E"

// Bootloader signatures
#define BOOTLOADER_SIGNATURE_GRUB "GRUB"
#define BOOTLOADER_SIGNATURE_LILO "LILO"
#define BOOTLOADER_SIGNATURE_SYSLINUX "SYSLINUX"
#define BOOTLOADER_SIGNATURE_ISOLINUX "ISOLINUX"
#define BOOTLOADER_SIGNATURE_PXELINUX "PXELINUX"

int chainload_mbr(const char* device_path, int partition);
int chainload_file(const char* bootloader_path);
int chainload_gpt(const char* device_path, int partition);
int chainload_iso(const char* iso_path);
int chainload_verify_bootloader(const char* bootloader_path);
int boot_chainload_kernel(uint8_t* bootloader_data, uint32_t bootloader_size);

#endif // BLOODHORN_CHAINLOAD_H 