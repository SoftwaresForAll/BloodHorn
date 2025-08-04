/*
 * BloodHorn Bootloader
 *
 * This file is part of BloodHorn and is licensed under the MIT License.
 * See the root of the repository for license details.
 */
#include <stdint.h>
#include "compat.h"
#include <string.h>
#include "chainload.h"

extern int read_sector(uint32_t lba, uint8_t* buf); // made it an int cuz IT NEEDS A ARTHIMATIC TYPE U BUFFON
extern int load_file(const char* path, uint8_t** data, uint32_t* size);

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

int chainload_mbr(const char* device_path, int partition) {
    uint8_t mbr_data[512];
    
    // Read MBR from device
    if (read_sector(0, mbr_data) != 0) {
        return -1;
    }
    
    struct mbr* mbr = (struct mbr*)mbr_data;
    
    // Verify MBR signature
    if (mbr->signature != 0xAA55) {
        return -1;
    }
    
    // Check if partition is valid
    if (partition < 0 || partition >= 4) {
        return -1;
    }
    
    struct mbr_partition* part = &mbr->partitions[partition];
    
    // Check if partition is active
    if (part->status != 0x80 && part->status != 0x00) {
        return -1;
    }
    
    // Check if partition has valid type
    if (part->type == 0x00) {
        return -1; // Empty partition
    }
    
    // Read boot sector from partition
    uint8_t boot_sector[512];
    if (read_sector(part->lba_start, boot_sector) != 0) {
        return -1;
    }
    
    // Verify boot sector signature
    if (boot_sector[510] != 0x55 || boot_sector[511] != 0xAA) {
        return -1;
    }
    
    // Load bootloader to 0x7C00
    memcpy((void*)0x7C00, boot_sector, 512);
    
    // Setup registers for bootloader
    // DL = drive number
    // DS:SI = partition table entry
    // ES:BX = boot sector address
    
    // Jump to bootloader
    void (*bootloader_entry)(void) = (void*)0x7C00;
    bootloader_entry();
    
    return 0;
}

int chainload_file(const char* bootloader_path) {
    uint8_t* bootloader_data = NULL;
    uint32_t bootloader_size = 0;
    
    // Load bootloader file
    if (load_file(bootloader_path, &bootloader_data, &bootloader_size) != 0) {
        return -1;
    }
    
    // Check if it's a valid bootloader
    if (bootloader_size < 512) {
        return -1;
    }
    
    // Verify boot sector signature
    if (bootloader_data[510] != 0x55 || bootloader_data[511] != 0xAA) {
        return -1;
    }
    
    // Load bootloader to 0x7C00
    memcpy((void*)0x7C00, bootloader_data, 512);
    
    // If bootloader is larger than 512 bytes, load the rest to high memory
    if (bootloader_size > 512) {
        uint32_t remaining_size = bootloader_size - 512;
        uint32_t high_mem_addr = 0x1000; // 4KB
        
        memcpy((void*)high_mem_addr, bootloader_data + 512, remaining_size);
    }
    
    // Jump to bootloader
    void (*bootloader_entry)(void) = (void*)0x7C00;
    bootloader_entry();
    
    return 0;
}

int chainload_gpt(const char* device_path, int partition) {
    uint8_t gpt_header_data[512];
    
    // Read GPT header from LBA 1
    if (read_sector(1, gpt_header_data) != 0) {
        return -1;
    }
    
    struct gpt_header* gpt_header = (struct gpt_header*)gpt_header_data;
    
    // Verify GPT signature
    if (gpt_header->signature != 0x5452415020494645) { // "EFI PART"
        return -1;
    }
    
    // Check if partition is valid
    if (partition < 0 || partition >= gpt_header->num_partition_entries) {
        return -1;
    }
    
    // Read partition entry
    uint8_t partition_entry_data[512];
    uint32_t entry_lba = gpt_header->partition_entry_lba + (partition * gpt_header->size_of_partition_entry) / 512;
    uint32_t entry_offset = (partition * gpt_header->size_of_partition_entry) % 512;
    
    if (read_sector(entry_lba, partition_entry_data) != 0) {
        return -1;
    }
    
    struct gpt_partition* gpt_part = (struct gpt_partition*)(partition_entry_data + entry_offset);
    
    // Check if partition is valid
    if (gpt_part->starting_lba == 0 || gpt_part->ending_lba == 0) {
        return -1;
    }
    
    // Read boot sector from partition
    uint8_t boot_sector[512];
    if (read_sector(gpt_part->starting_lba, boot_sector) != 0) {
        return -1;
    }
    
    // Verify boot sector signature
    if (boot_sector[510] != 0x55 || boot_sector[511] != 0xAA) {
        return -1;
    }
    
    // Load bootloader to 0x7C00
    memcpy((void*)0x7C00, boot_sector, 512);
    
    // Jump to bootloader
    void (*bootloader_entry)(void) = (void*)0x7C00;
    bootloader_entry();
    
    return 0;
}

int chainload_iso(const char* iso_path) {
    uint8_t* iso_data = NULL;
    uint32_t iso_size = 0;
    
    // Load ISO file
    if (load_file(iso_path, &iso_data, &iso_size) != 0) {
        return -1;
    }
    
    // Check if it's a valid ISO
    if (iso_size < 32768) { // At least 16 sectors
        return -1;
    }
    
    // Read boot sector from ISO (sector 16)
    uint8_t boot_sector[2048];
    memcpy(boot_sector, iso_data + 16 * 2048, 2048);
    
    // Check for El Torito boot signature
    if (memcmp(boot_sector + 1, "CD001", 5) != 0) {
        return -1;
    }
    
    // Load bootloader to 0x7C00
    memcpy((void*)0x7C00, boot_sector, 2048);
    
    // Jump to bootloader
    void (*bootloader_entry)(void) = (void*)0x7C00;
    bootloader_entry();
    
    return 0;
}

int chainload_verify_bootloader(const char* bootloader_path) {
    uint8_t* bootloader_data = NULL;
    uint32_t bootloader_size = 0;
    
    if (load_file(bootloader_path, &bootloader_data, &bootloader_size) != 0) {
        return -1;
    }
    
    // Check minimum size
    if (bootloader_size < 512) {
        return -1;
    }
    
    // Check boot sector signature
    if (bootloader_data[510] != 0x55 || bootloader_data[511] != 0xAA) {
        return -1;
    }
    
    // Check for common bootloader signatures
    if (memcmp(bootloader_data + 3, "GRUB", 4) == 0) {
        return 0; // GRUB
    }
    
    if (memcmp(bootloader_data + 3, "LILO", 4) == 0) {
        return 0; // LILO
    }
    
    if (memcmp(bootloader_data + 3, "SYSLINUX", 8) == 0) {
        return 0; // SYSLINUX
    }
    
    // Generic valid bootloader
    return 0;
} 

int boot_chainload_kernel(uint8_t* bootloader_data, uint32_t bootloader_size) {
    uint8_t* bootloader_dest = (uint8_t*)0x7C00;
    memcpy(bootloader_dest, bootloader_data, bootloader_size);
    
    void (*bootloader_entry)(void) = (void*)0x7C00;
    bootloader_entry();
    
    return 0;
} 