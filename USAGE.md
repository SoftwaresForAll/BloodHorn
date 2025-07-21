# BloodHorn Usage Guide

This guide explains how to use BloodHorn bootloader with different operating systems and custom kernels.

## Table of Contents

1. [Basic Usage](#basic-usage)
2. [Linux Systems](#linux-systems)
3. [Windows Systems](#windows-systems)
4. [macOS Systems](#macos-systems)
5. [Custom Kernels](#custom-kernels)
6. [Network Boot](#network-boot)
7. [Configuration Files](#configuration-files)
8. [Scripting](#scripting)
9. [Recovery Mode](#recovery-mode)

## Basic Usage

### Starting BloodHorn

1. **BIOS Boot**: BloodHorn will automatically start if installed as the primary bootloader
2. **UEFI Boot**: Select BloodHorn from the UEFI boot menu
3. **USB Boot**: Boot from a USB drive containing BloodHorn

### Navigation

- **Arrow Keys**: Navigate menu entries
- **Enter**: Boot selected entry
- **ESC**: Exit to firmware
- **Mouse**: Click to select entries (if supported)
- **Hotkeys**: Press number keys for quick access

## Linux Systems

BloodHorn supports multiple Linux boot protocols:

### Native Linux Boot (bzImage)

```bash
# Boot Ubuntu kernel using native Linux protocol
# Add to BloodHorn menu:
# Title: Ubuntu 22.04 (Native)
# Type: Linux
# Kernel: /boot/vmlinuz-5.15.0-generic
# Initrd: /boot/initrd.img-5.15.0-generic
# Cmdline: root=/dev/sda1 ro quiet splash

# Boot without initrd
# Title: Ubuntu Minimal
# Type: Linux
# Kernel: /boot/vmlinuz-5.15.0-generic
# Cmdline: root=/dev/sda1 ro
```

### Multiboot2 Linux Boot

```bash
# Boot using Multiboot2 protocol
# Add to BloodHorn menu:
# Title: Ubuntu 22.04 (Multiboot2)
# Type: Multiboot2
# Kernel: /boot/vmlinuz-mb2
# Initrd: /boot/initrd.img-5.15.0-generic
# Cmdline: root=/dev/sda1 ro quiet splash
```

### Limine Protocol Linux Boot

```bash
# Boot using Limine protocol
# Add to BloodHorn menu:
# Title: Ubuntu 22.04 (Limine)
# Type: Limine
# Kernel: /boot/vmlinuz-limine
# Initrd: /boot/initrd.img-5.15.0-generic
# Cmdline: root=/dev/sda1 ro quiet splash
```

### Ubuntu/Debian Installation

```bash
# Install BloodHorn
sudo cp build/BloodHorn.efi /boot/efi/EFI/BOOT/BOOTX64.EFI

# Create boot entry
sudo efibootmgr -c -d /dev/sda -p 1 -l "\\EFI\\BOOT\\BOOTX64.EFI" -L "BloodHorn"
```

### Arch Linux

```bash
# Install BloodHorn
sudo cp build/BloodHorn.efi /boot/efi/EFI/BOOT/BOOTX64.EFI

# Boot Arch kernel
# Add to BloodHorn menu:
# Title: Arch Linux
# Kernel: /boot/vmlinuz-linux
# Initrd: /boot/initramfs-linux.img
# Cmdline: root=/dev/sda2 rw
```

### Custom Linux Distribution

```bash
# Boot custom Linux kernel
# Add to BloodHorn menu:
# Title: Custom Linux
# Kernel: /boot/custom-kernel
# Initrd: /boot/custom-initrd
# Cmdline: root=/dev/sda3 rw console=ttyS0
```

## Windows Systems

### Windows 10/11

```bash
# Install BloodHorn alongside Windows
sudo cp build/BloodHorn.efi /boot/efi/EFI/BOOT/BOOTX64.EFI

# Chainload Windows Boot Manager
# Add to BloodHorn menu:
# Title: Windows 11
# Type: Chainloader
# Path: /boot/efi/EFI/Microsoft/Boot/bootmgfw.efi
```

## Chainloading

BloodHorn supports chainloading other bootloaders and operating systems.

### MBR Chainloading

```bash
# Chainload from MBR partition
# Add to BloodHorn menu:
# Title: GRUB Legacy
# Type: Chainload MBR
# Device: /dev/sda
# Partition: 1
```

### GPT Chainloading

```bash
# Chainload from GPT partition
# Add to BloodHorn menu:
# Title: GRUB2 GPT
# Type: Chainload GPT
# Device: /dev/sda
# Partition: 2
```

### File-based Chainloading

```bash
# Chainload bootloader file
# Add to BloodHorn menu:
# Title: Custom Bootloader
# Type: Chainload File
# Path: /boot/custom-bootloader.bin
```

### ISO Chainloading

```bash
# Chainload from ISO file
# Add to BloodHorn menu:
# Title: Live CD
# Type: Chainload ISO
# Path: /boot/live-cd.iso
```

### Common Bootloaders

```bash
# GRUB2
# Title: GRUB2
# Type: Chainload File
# Path: /boot/grub/i386-pc/core.img

# LILO
# Title: LILO
# Type: Chainload MBR
# Device: /dev/sda
# Partition: 1

# SYSLINUX
# Title: SYSLINUX
# Type: Chainload File
# Path: /boot/syslinux/ldlinux.sys
```

### Windows Server

```bash
# Boot Windows Server
# Add to BloodHorn menu:
# Title: Windows Server 2022
# Type: Chainloader
# Path: /boot/efi/EFI/Microsoft/Boot/bootmgfw.efi
# Cmdline: /safeboot:minimal
```

## macOS Systems

### macOS (Intel Macs)

```bash
# Install BloodHorn on Intel Mac
sudo cp build/BloodHorn.efi /System/Volumes/Preboot/*/System/Library/CoreServices/boot.efi

# Chainload macOS
# Add to BloodHorn menu:
# Title: macOS Monterey
# Type: Chainloader
# Path: /System/Library/CoreServices/boot.efi
```

### macOS (Apple Silicon)

```bash
# Note: Apple Silicon Macs require different approach
# Use OpenCore or similar for Apple Silicon compatibility
```

## Custom Kernels

### Multiboot2 Kernel

```c
// Example Multiboot2 kernel
#include <multiboot2.h>

void kernel_main(uint32_t magic, struct multiboot_tag* tag) {
    // Your kernel code here
}

// Compile with:
// gcc -c kernel.c -o kernel.o
// ld -T kernel.ld -o kernel.bin kernel.o
// Add to BloodHorn menu:
// Title: Custom Multiboot2 Kernel
// Type: Multiboot2
// Path: /boot/custom-kernel.bin
```

### ELF Kernel

```c
// Example ELF kernel
void _start(void) {
    // Initialize kernel
    kernel_init();
    
    // Start user space
    start_userspace();
}

// Add to BloodHorn menu:
// Title: Custom ELF Kernel
// Type: ELF
// Path: /boot/custom-kernel.elf
```

### Custom Boot Protocol

```c
// Example custom boot protocol
struct custom_boot_info {
    uint32_t magic;
    uint32_t version;
    void* kernel_start;
    void* kernel_end;
    void* stack_top;
};

// Add to BloodHorn menu:
// Title: Custom Protocol Kernel
// Type: Custom
// Path: /boot/custom-kernel.bin
// Protocol: custom_boot
```

## Network Boot

### PXE Boot Setup

```bash
# Configure DHCP server (dnsmasq example)
cat > /etc/dnsmasq.conf << EOF
interface=eth0
dhcp-range=192.168.1.100,192.168.1.200,12h
dhcp-boot=pxelinux.0
enable-tftp
tftp-root=/var/lib/tftpboot
EOF

# Place kernel in TFTP root
sudo cp kernel.bin /var/lib/tftpboot/

# BloodHorn will automatically:
# 1. Send DHCPDISCOVER
# 2. Receive DHCPOFFER with TFTP server info
# 3. Download kernel via TFTP
# 4. Boot the downloaded kernel
```

### HTTP Boot

```bash
# Set up HTTP server
python3 -m http.server 8080

# Place kernel in web root
cp kernel.bin /var/www/html/

# BloodHorn will download kernel via HTTP
# Add to BloodHorn menu:
# Title: HTTP Boot Kernel
# Type: HTTP
# URL: http://192.168.1.10:8080/kernel.bin
```

## Configuration Files

### INI Configuration

```ini
# /boot/bloodhorn.ini
[theme]
background_color=0x1A1A2E
text_color=0xCCCCCC
highlight_color=0x4A4A8A

[menu]
timeout=10
default_entry=0

[entry:0]
title=Ubuntu 22.04
type=kernel
kernel=/boot/vmlinuz-5.15.0-generic
initrd=/boot/initrd.img-5.15.0-generic
cmdline=root=/dev/sda1 ro quiet splash

[entry:1]
title=Windows 11
type=chainloader
path=/boot/efi/EFI/Microsoft/Boot/bootmgfw.efi

[entry:2]
title=Custom Kernel
type=multiboot2
kernel=/boot/custom-kernel.bin
```

### JSON Configuration

```json
{
  "theme": {
    "background_color": "0x1A1A2E",
    "text_color": "0xCCCCCC",
    "highlight_color": "0x4A4A8A"
  },
  "menu": {
    "timeout": 10,
    "default_entry": 0
  },
  "entries": [
    {
      "title": "Ubuntu 22.04",
      "type": "kernel",
      "kernel": "/boot/vmlinuz-5.15.0-generic",
      "initrd": "/boot/initrd.img-5.15.0-generic",
      "cmdline": "root=/dev/sda1 ro quiet splash"
    },
    {
      "title": "Windows 11",
      "type": "chainloader",
      "path": "/boot/efi/EFI/Microsoft/Boot/bootmgfw.efi"
    }
  ]
}
```

## Scripting

### Lua Scripts

```lua
-- /boot/bloodhorn.lua
function on_boot()
    -- Check if network is available
    if network_available() then
        -- Download latest kernel
        download_kernel("http://server/kernel.bin")
    end
    
    -- Verify kernel signature
    if verify_signature("/boot/kernel.bin", "/boot/kernel.sig") then
        boot_kernel("/boot/kernel.bin")
    else
        show_error("Kernel signature verification failed")
    end
end

function on_menu_select(entry)
    -- Custom menu selection logic
    if entry.title == "Safe Mode" then
        entry.cmdline = entry.cmdline .. " single"
    end
end
```

### Script Integration

```bash
# BloodHorn will automatically load and execute:
# - /boot/bloodhorn.lua (main script)
# - /boot/scripts/*.lua (additional scripts)
```

## Recovery Mode

### Accessing Recovery Shell

1. **From Menu**: Select "Recovery Mode" from boot menu
2. **Hotkey**: Press 'R' during boot
3. **Automatic**: BloodHorn enters recovery if kernel fails to load

### Recovery Commands

```bash
bloodhorn> help
Available commands:
  help     - Show this help
  ls       - List files
  cat <file> - Show file contents
  mount <device> <path> - Mount filesystem
  boot <kernel> - Boot kernel manually
  reboot   - Reboot system
  clear    - Clear screen

bloodhorn> ls /boot
vmlinuz-5.15.0-generic
initrd.img-5.15.0-generic
bloodhorn.ini

bloodhorn> cat /boot/bloodhorn.ini
[menu]
timeout=10
...

bloodhorn> boot /boot/vmlinuz-5.15.0-generic
Booting kernel...
```

### Troubleshooting

```bash
# Check filesystem
bloodhorn> mount /dev/sda1 /mnt
bloodhorn> ls /mnt/boot

# Verify kernel
bloodhorn> verify_signature /boot/kernel.bin /boot/kernel.sig

# Check network
bloodhorn> ping 8.8.8.8

# View logs
bloodhorn> cat /var/log/bloodhorn.log
```

## Advanced Usage

### Custom Boot Protocols

```c
// Define custom boot protocol
struct custom_boot_info {
    uint32_t magic;
    uint32_t version;
    void* kernel_start;
    void* kernel_end;
    void* stack_top;
    void* heap_start;
    void* heap_end;
};

// Register with BloodHorn
register_boot_protocol("custom", custom_boot_handler);
```

### Plugin Development

```c
// Example plugin
#include "plugins/plugin.h"

void my_plugin_init(void) {
    // Initialize plugin
    printf("My plugin loaded\n");
}

void my_plugin_cleanup(void) {
    // Cleanup plugin
    printf("My plugin unloaded\n");
}

// Register plugin
PLUGIN_REGISTER("my_plugin", "1.0", my_plugin_init, my_plugin_cleanup);
```

### Security Features

```bash
# Enable Secure Boot verification
# Add to configuration:
[security]
secure_boot=enabled
verify_signatures=true
allowed_keys=/boot/keys/

# Sign kernel
openssl dgst -sha256 -sign private.key -out kernel.sig kernel.bin

# BloodHorn will verify signature before booting
```

This usage guide covers the most common scenarios. For advanced usage, refer to the source code and API documentation. 