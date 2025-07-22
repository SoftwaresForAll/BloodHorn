# BloodHorn
<p align="center">
  <img src="Zeak.png" alt="Project Logo" width="200">
</p>



BloodHorn is a modern bootloader supporting Linux, Multiboot, Limine, PXE, and multiple CPU architectures.

---
## Donations and community support

#### If you intend to support the project maintainer (Aka [@sparkelez](https://discordapp.com/users/1351825562791841832)), you can do so by donating. Even the smallest donation can make a difference.

[![Donate](https://liberapay.com/assets/widgets/donate.svg)](https://liberapay.com/Listedroot/donate)

## Features
- Linux boot
- Multiboot 1
- Multiboot 2
- Limine
- Chainloading
- PXE network boot
- FAT32
- ext2
- ISO9660
- SHA256
- RSA
- AES
- HMAC
- Secure boot
- Entropy source
- Graphical menu (theming, background image, localization)
- Mouse support
- Lua scripting
- Recovery shell
- Plugin system
- Modular network stack (DHCP, TFTP, ARP)
- Modular filesystem stack

## Architectures
- IA-32 (x86)
- x86-64
- aarch64 (arm64)
- riscv64
- loongarch64

## Boot Protocols
- Linux
- Multiboot 1
- Multiboot 2
- [Limine](https://github.com/limine-bootloader/limine-protocol/blob/trunk/PROTOCOL.md)
- Chainloading
- PXE

## Configuration
- INI, JSON, and environment variable support
- Theme and language options (see CONFIG.md)
- Place config files in the root of the boot partition

## Quick Start
```
make
make test
```

## Why We Don't Provide Binaries
BloodHorn does not release pre-built binaries. This is to ensure maximum security, transparency, and trust:
- Building from source guarantees you know exactly what code is running on your system.
- Bootloaders are highly security-sensitive; you should always verify and build your own binary.
- Different systems and firmware may require different build options or configurations.
- This approach encourages review, customization, and community contributions.

See INSTALL.md and USAGE.md for build and usage instructions.

#### *BloodHorn was inspired by modern bootloaders, but all code is original and written from scratch and it's made originally for fun and to be used in my future operating systems!.*
## License
MIT

