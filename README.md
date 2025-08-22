
# BloodHorn
<p align="center">
  <img src="Zeak.png" alt="Project Logo" width="200">
</p>

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![EDK2 Compatible](https://img.shields.io/badge/EDK2-Compatible-blue)](https://github.com/tianocore/tianocore.github.io/)
[![UEFI](https://img.shields.io/badge/UEFI-Secure%20Boot-0078D7)](https://uefi.org/)
[![Architectures](https://img.shields.io/badge/Arch-x86__64%20%7C%20ARM64%20%7C%20RISC--V-0078D7)](https://en.wikipedia.org/wiki/Unified_Extensible_Firmware_Interface)
[![Build Status](https://github.com/Listedroot/BloodHorn/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/Listedroot/BloodHorn/actions)

BloodHorn is a modern bootloader supporting Linux, Multiboot, Limine, PXE, and multiple CPU architectures.

---
## Build Requirements

- GCC or Clang
- NASM
- EDK2 (for UEFI builds)
- Python 3.6+

## Security Notice
**BloodHorn is a security-sensitive project. Always build from source and review code before use. Never trust pre-built binaries for bootloaders.**

## Quick Build

### Prerequisites (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y build-essential uuid-dev nasm acpica-tools \
    python3-full qemu-system-x86 ovmf git gcc-aarch64-linux-gnu \
    gcc-riscv64-linux-gnu
```

### Building with EDK2
```bash
# Clone EDK2
git clone --depth 1 https://github.com/tianocore/edk2.git
cd edk2
git submodule update --init

# Build base tools
make -C BaseTools

# Set up environment
export EDK_TOOLS_PATH=$(pwd)/BaseTools
export PACKAGES_PATH=$(pwd):$(pwd)/..
. edksetup.sh

# Build BloodHorn (from the repository root)
cp -r ../BloodHorn .
build -a X64 -p BloodHorn.dsc -t GCC5

# Run in QEMU
qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom BloodHorn.iso
```


## Features

- Linux boot
- Multiboot 1 & 2
- Limine protocol
- Chainloading
- PXE network boot
- FAT32, ext2, ISO9660 support
- SHA256, RSA, AES, HMAC
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
- [BloodChain](docs/BloodChain-Protocol.md) 
- Chainloading
- PXE


## Configuration

- INI, JSON, and environment variable support
- Theme and language options (see CONFIG.md)
- Place config files in the root of the boot partition


## Quick Start

```bash
make
make test
```


## Why We Don't Provide Binaries

BloodHorn does not release pre-built binaries. This is to ensure maximum security, transparency, and trust:

- Building from source guarantees you know exactly what code is running on your system.
- Bootloaders are highly security-sensitive; you should always verify and build your own binary.
- Different systems and firmware may require different build options or configurations.
- This approach encourages review, customization, and community contributions.

## ✨ Contributors

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/PacHashs">
        <img src="https://avatars.githubusercontent.com/PacHashs" width="120px;" alt="My avatar"/>
        <br />
        <sub><b>PacHash</b></sub>
      </a>
      <br />
      Lead Developer 
    </td>
    <td align="center">
      <a href="https://github.com/unpopularopinionn">
        <img src="https://avatars.githubusercontent.com/unpopularopinionn" width="120px;" alt="Homie avatar"/>
        <br />
        <sub><b>UnpopularOpinion</b></sub>
      </a>
      <br />
      Main Team
    </td>
  </tr>
</table>


See INSTALL.md and USAGE.md for build and usage instructions.

---

## License

This project is licensed under the MIT License. See the LICENSE file for details.

---

#### *BloodHorn was inspired by modern bootloaders, but all code is original and written from scratch and it's made originally for fun and to be used in my future operating systems!*

## Special Thanks

We would like to extend our deepest gratitude to the organizations and communities that have significantly influenced, inspired, and supported the development of this project:

* **[OSDev.org](https://wiki.osdev.org/Main_Page)** – For their comprehensive documentation and tutorials on low-level system programming, bootloader design, and operating system development. Your guides made complex concepts approachable and actionable.
* **[GNU Project](https://www.gnu.org/)** – For providing essential free software tools, compilers, and libraries that make cross-platform development possible.
* **[Linux Foundation](https://www.linuxfoundation.org/)** – For fostering an open-source ecosystem and for the invaluable resources on kernel development, boot processes, and system internals.
* **[U-Boot Community](https://www.denx.de/wiki/U-Boot)** – For their work on one of the most widely used open-source bootloaders. The concepts and codebase serve as a reference for embedded bootloader design.
* **[Intel Developer Zone](https://software.intel.com/)** – For providing documentation on x86 architecture, chipset manuals, and optimization techniques crucial for low-level programming.
* **[ARM Developer](https://developer.arm.com/)** – For offering in-depth guides and resources for ARM architecture boot processes, which have been instrumental for cross-platform compatibility.
* **[Stack Overflow](https://stackoverflow.com/)** – For being an invaluable community where countless bootloader, assembly, and system programming challenges were solved through shared knowledge.
* **[The OSDev Community Forums](https://forum.osdev.org/)** – For discussions, peer reviews, and troubleshooting tips that helped refine this project from concept to implementation.
* **[GitHub Open Source Contributors](https://github.com/)** – For inspiring modern coding practices and maintaining a culture of sharing knowledge, which allowed this bootloader project to thrive.

We also acknowledge all **open-source developers** whose contributions, code samples, and documentation have indirectly made this project possible. Your dedication to the craft of software development drives innovation and education in the low-level programming space.
