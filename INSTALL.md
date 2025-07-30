# BloodHorn Installation Guide

This guide explains how to install BloodHorn bootloader on different systems and platforms.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Building BloodHorn](#building-bloodhorn)
3. [UEFI Installation](#uefi-installation)
4. [BIOS Installation](#bios-installation)
5. [USB Installation](#usb-installation)
6. [Network Installation](#network-installation)
7. [Platform-Specific Installation](#platform-specific-installation)
8. [Troubleshooting](#troubleshooting)

## Prerequisites

### Required Software

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential nasm gcc-multilib gcc-mingw-w64 \
    genisoimage qemu-system-x86 ovmf mtools

# CentOS/RHEL/Fedora
sudo dnf groupinstall "Development Tools"
sudo dnf install nasm mingw64-gcc genisoimage qemu ovmf mtools

# Arch Linux
sudo pacman -S base-devel nasm mingw-w64-gcc cdrtools qemu ovmf mtools

# macOS
brew install gcc nasm qemu mtools
```

### Hardware Requirements

- **CPU**: x86_64 compatible processor
- **Memory**: 512MB RAM minimum, 2GB recommended
- **Storage**: 100MB free space for installation
- **Boot Method**: UEFI or BIOS support

## Building BloodHorn

### Clone and Build

```bash
# Clone repository
git clone https://github.com/Listedroot/BloodHorn.git
cd BloodHorn

# Install dependencies (Ubuntu/Debian example)
sudo apt update
sudo apt install -y build-essential uuid-dev nasm acpica-tools \
    python3-full qemu-system-x86 ovmf git gcc-aarch64-linux-gnu \
    gcc-riscv64-linux-gnu

# Clone and set up EDK2
git clone --depth 1 https://github.com/tianocore/edk2.git
cd edk2
git submodule update --init
make -C BaseTools

export EDK_TOOLS_PATH=$(pwd)/BaseTools
export PACKAGES_PATH=$(pwd):$(pwd)/..
. edksetup.sh

# Copy and build BloodHorn
cp -r ../BloodHorn .
build -a X64 -p BloodHorn.dsc -t GCC5

# Create bootable ISO
mkdir -p output/EFI/BOOT
cp Build/BloodHorn/DEBUG_GCC5/X64/BloodHorn.efi output/EFI/BOOT/BOOTX64.EFI
xorriso -as mkisofs -e EFI/BOOT/BOOTX64.EFI -no-emul-boot -o ../BloodHorn.iso output/
cd ..
# Verify build
ls -la build/
# Should show:
# - build/BloodHorn.efi (UEFI version)
# - build/bios/bootsector.bin (BIOS boot sector)
# - build/bios/stage2.bin (BIOS stage 2)
# - iso/BloodHorn.iso (Bootable ISO)
```

### Build Options

```bash
# Build UEFI version (from edk2 directory)
build -a X64 -p BloodHorn.dsc -t GCC5

# Create bootable ISO
mkdir -p output/EFI/BOOT
cp Build/BloodHorn/DEBUG_GCC5/X64/BloodHorn.efi output/EFI/BOOT/BOOTX64.EFI
xorriso -as mkisofs -e EFI/BOOT/BOOTX64.EFI -no-emul-boot -o BloodHorn.iso output/

# Clean build artifacts
rm -rf Build/* output/ BloodHorn.iso
```

## UEFI Installation

### Method 1: EFI System Partition (Recommended)

```bash
# 1. Mount EFI partition
sudo mount /dev/sda1 /mnt/efi  # Adjust device as needed

# 2. Create BloodHorn directory
sudo mkdir -p /mnt/efi/EFI/BloodHorn

# 3. Copy BloodHorn
sudo cp build/BloodHorn.efi /mnt/efi/EFI/BloodHorn/

# 4. Create boot entry
sudo efibootmgr -c -d /dev/sda -p 1 -l "\\EFI\\BloodHorn\\BloodHorn.efi" -L "BloodHorn"

# 5. Set as default (optional)
sudo efibootmgr -o 0000,0001,0002  # BloodHorn first

# 6. Unmount
sudo umount /mnt/efi
```

### Method 2: Replace Default Bootloader

```bash
# 1. Backup existing bootloader
sudo cp /boot/efi/EFI/BOOT/BOOTX64.EFI /boot/efi/EFI/BOOT/BOOTX64.EFI.backup

# 2. Install BloodHorn as default
sudo cp build/BloodHorn.efi /boot/efi/EFI/BOOT/BOOTX64.EFI

# 3. Reboot to test
sudo reboot
```

### Method 3: Dual Boot with Windows

```bash
# 1. Boot into Windows
# 2. Open Command Prompt as Administrator
# 3. Create BloodHorn directory
mkdir C:\EFI\BloodHorn

# 4. Copy BloodHorn (from Linux USB or network)
copy BloodHorn.efi C:\EFI\BloodHorn\

# 5. Add boot entry
bcdedit /create /d "BloodHorn" /application bootapp
bcdedit /set {GUID} path \EFI\BloodHorn\BloodHorn.efi
bcdedit /displayorder {GUID} /addlast
```

## BIOS Installation

### Method 1: MBR Installation

```bash
# 1. Backup MBR
sudo dd if=/dev/sda of=/backup/mbr.backup bs=512 count=1

# 2. Install BloodHorn boot sector
sudo dd if=build/bios/bootsector.bin of=/dev/sda bs=512 count=1

# 3. Copy stage 2 loader
sudo dd if=build/bios/stage2.bin of=/dev/sda bs=512 seek=1

# 4. Reboot to test
sudo reboot
```

### Method 2: Partition Boot Sector

```bash
# 1. Install to specific partition
sudo dd if=build/bios/bootsector.bin of=/dev/sda1 bs=512 count=1

# 2. Copy stage 2 to partition
sudo dd if=build/bios/stage2.bin of=/dev/sda1 bs=512 seek=1

# 3. Set partition as bootable
sudo fdisk /dev/sda
# Use 'a' to toggle boot flag on partition 1
```

## USB Installation

### Create Bootable USB

```bash
# 1. Identify USB device (be careful!)
lsblk
# Example: /dev/sdb

# 2. Create bootable USB
sudo dd if=iso/BloodHorn.iso of=/dev/sdb bs=4M status=progress

# 3. Sync and eject
sudo sync
sudo eject /dev/sdb
```

### Manual USB Setup

```bash
# 1. Format USB as FAT32
sudo mkfs.fat -F 32 /dev/sdb1

# 2. Mount USB
sudo mount /dev/sdb1 /mnt/usb

# 3. Create EFI structure
sudo mkdir -p /mnt/usb/EFI/BOOT

# 4. Copy BloodHorn
sudo cp build/BloodHorn.efi /mnt/usb/EFI/BOOT/BOOTX64.EFI

# 5. Copy configuration
sudo cp bloodhorn.ini /mnt/usb/

# 6. Unmount
sudo umount /mnt/usb
```

## Network Installation

### PXE Server Setup

```bash
# 1. Install DHCP/TFTP server
sudo apt install dnsmasq

# 2. Configure dnsmasq
sudo tee /etc/dnsmasq.conf << EOF
interface=eth0
dhcp-range=192.168.1.100,192.168.1.200,12h
dhcp-boot=BloodHorn.efi
enable-tftp
tftp-root=/var/lib/tftpboot
EOF

# 3. Create TFTP directory
sudo mkdir -p /var/lib/tftpboot

# 4. Copy BloodHorn to TFTP
sudo cp build/BloodHorn.efi /var/lib/tftpboot/

# 5. Start services
sudo systemctl enable dnsmasq
sudo systemctl start dnsmasq
```

### HTTP Boot Server

```bash
# 1. Install web server
sudo apt install nginx

# 2. Create web directory
sudo mkdir -p /var/www/boot

# 3. Copy BloodHorn
sudo cp build/BloodHorn.efi /var/www/boot/

# 4. Configure nginx
sudo tee /etc/nginx/sites-available/boot << EOF
server {
    listen 80;
    server_name boot.local;
    root /var/www/boot;
    location / {
        try_files \$uri \$uri/ =404;
    }
}
EOF

# 5. Enable site
sudo ln -s /etc/nginx/sites-available/boot /etc/nginx/sites-enabled/
sudo systemctl restart nginx
```

## Platform-Specific Installation

### Linux Systems

#### Ubuntu/Debian

```bash
# Install BloodHorn package
sudo apt install bloodhorn

# Or manual installation
sudo cp build/BloodHorn.efi /boot/efi/EFI/BOOT/BOOTX64.EFI
sudo update-grub  # Update GRUB to include BloodHorn
```

#### Arch Linux

```bash
# Install from AUR
yay -S bloodhorn

# Or manual installation
sudo cp build/BloodHorn.efi /boot/efi/EFI/BOOT/BOOTX64.EFI
sudo grub-mkconfig -o /boot/grub/grub.cfg
```

#### CentOS/RHEL/Fedora

```bash
# Install BloodHorn
sudo dnf install bloodhorn

# Or manual installation
sudo cp build/BloodHorn.efi /boot/efi/EFI/BOOT/BOOTX64.EFI
sudo grub2-mkconfig -o /boot/grub2/grub.cfg
```

### Windows Systems

#### Windows 10/11

```powershell
# Run PowerShell as Administrator

# 1. Create BloodHorn directory
New-Item -ItemType Directory -Path "C:\EFI\BloodHorn" -Force

# 2. Copy BloodHorn (from USB or network)
Copy-Item "BloodHorn.efi" "C:\EFI\BloodHorn\"

# 3. Add boot entry
bcdedit /create /d "BloodHorn" /application bootapp
bcdedit /set {GUID} path \EFI\BloodHorn\BloodHorn.efi
bcdedit /displayorder {GUID} /addlast
```

#### Windows Server

```powershell
# Similar to Windows 10/11, but with server-specific paths
# Install to C:\Windows\Boot\EFI\BloodHorn\
```

### macOS Systems

#### Intel Macs

```bash
# 1. Boot into Recovery Mode (Cmd+R)
# 2. Open Terminal
# 3. Disable SIP temporarily
csrutil disable

# 4. Mount EFI partition
sudo mkdir -p /Volumes/EFI
sudo mount -t msdos /dev/disk0s1 /Volumes/EFI

# 5. Install BloodHorn
sudo cp BloodHorn.efi /Volumes/EFI/EFI/BOOT/BOOTX64.EFI

# 6. Re-enable SIP
csrutil enable

# 7. Reboot
sudo reboot
```

#### Apple Silicon Macs

```bash
# Note: Apple Silicon requires different approach
# Use OpenCore or similar for compatibility
# BloodHorn may need modifications for Apple Silicon
```

## Configuration

### Basic Configuration

You can customize the boot menu appearance and language using INI or JSON config files. Place `bloodhorn.ini` or `bloodhorn.json` in the root of your boot partition (e.g., `/boot/`, `/mnt/usb/`, or EFI partition root).

#### Example INI:
```
[theme]
background_color=0x1A1A2E
header_color=0x2D2D4F
highlight_color=0x4A4A8A
text_color=0xCCCCCC
selected_text_color=0xFFFFFF
footer_color=0x8888AA
background_image=myimage.bmp

[localization]
language=en
```

#### Example JSON:
```
{
  "theme": {
    "background_color": "0x1A1A2E",
    "header_color": "0x2D2D4F",
    "highlight_color": "0x4A4A8A",
    "text_color": "0xCCCCCC",
    "selected_text_color": "0xFFFFFF",
    "footer_color": "0x8888AA",
    "background_image": "myimage.bmp"
  },
  "localization": {
    "language": "en"
  }
}
```

- Supported image formats for `background_image` depend on your build (BMP recommended).
- For localization, you can add external language files (e.g., `lang_en.txt`).
- If both INI and JSON exist, INI takes precedence.

### Advanced Configuration

```bash
# Security configuration
sudo tee /boot/bloodhorn-security.ini << EOF
[security]
secure_boot=enabled
verify_signatures=true
allowed_keys=/boot/keys/

[network]
pxe_enabled=true
http_enabled=true
tftp_server=192.168.1.10
EOF
```

## Troubleshooting

### Common Issues

#### BloodHorn Won't Boot

```bash
# 1. Check UEFI settings
# Enter UEFI setup (F2, Del, F12)
# Enable UEFI boot
# Disable Secure Boot (if needed)

# 2. Check boot order
sudo efibootmgr -v

# 3. Verify installation
ls -la /boot/efi/EFI/BOOT/BOOTX64.EFI

# 4. Check logs
dmesg | grep -i efi
```

#### Kernel Loading Fails

```bash
# 1. Check kernel path
ls -la /boot/vmlinuz*

# 2. Verify filesystem
sudo fsck /dev/sda1

# 3. Check kernel parameters
cat /proc/cmdline

# 4. Use recovery shell
# Boot into recovery mode and check manually
```

#### Network Boot Issues

```bash
# 1. Check network connectivity
ping 8.8.8.8

# 2. Verify DHCP server
sudo systemctl status dnsmasq

# 3. Check TFTP server
sudo netstat -tulpn | grep :69

# 4. Test TFTP manually
tftp 192.168.1.10
get BloodHorn.efi
quit
```

### Recovery Procedures

#### Reinstall BloodHorn

```bash
# 1. Boot from USB or network
# 2. Mount root filesystem
sudo mount /dev/sda1 /mnt

# 3. Reinstall BloodHorn
sudo cp BloodHorn.efi /mnt/boot/efi/EFI/BOOT/BOOTX64.EFI

# 4. Reboot
sudo reboot
```

#### Restore Original Bootloader

```bash
# 1. Restore GRUB (if using GRUB)
sudo grub-install /dev/sda
sudo update-grub

# 2. Restore Windows Boot Manager
# Boot from Windows recovery media
bootrec /fixmbr
bootrec /fixboot
bootrec /rebuildbcd
```

## Verification

### Test Installation

```bash
# 1. Build and test in QEMU
make test

# 2. Test on real hardware
# Boot from USB first to verify

# 3. Check boot logs
dmesg | grep -i bloodhorn

# 4. Verify configuration
bloodhorn --verify-config /boot/bloodhorn.ini
```

### Security Verification

```bash
# 1. Verify Secure Boot
mokutil --sb-state

# 2. Check signatures
openssl dgst -verify public.key -signature kernel.sig kernel.bin

# 3. Verify BloodHorn integrity
sha256sum build/BloodHorn.efi
```

This installation guide covers the most common scenarios. For advanced installation options, refer to the source code and API documentation. 