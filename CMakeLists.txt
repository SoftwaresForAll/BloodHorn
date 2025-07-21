cmake_minimum_required(VERSION 3.10)
project(BloodHorn VERSION 1.0.0 LANGUAGES C ASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fno-stack-protector -fno-pie -no-pie")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -f elf64")

# Source files
set(SOURCES
    main.c
    boot/menu.c
    boot/theme.c
    boot/localization.c
    boot/mouse.c
    boot/secure.c
    boot/Arch32/linux.c
    boot/Arch32/limine.c
    boot/Arch32/multiboot1.c
    boot/Arch32/multiboot2.c
    boot/Arch32/chainload.c
    uefi/graphics.c
    uefi/uefi.c
    fs/fat32.c
    fs/ext2.c
    net/pxe.c
    security/crypto.c
    scripting/lua.c
    recovery/shell.c
    plugins/plugin.c
    config/config_ini.c
    multiboot/multiboot2_loader.c
)

# BIOS build
add_executable(BloodHorn-bios ${SOURCES})
set_target_properties(BloodHorn-bios PROPERTIES
    SUFFIX ".bin"
    LINK_FLAGS "-nostdlib -static"
)

# UEFI build
add_executable(BloodHorn-uefi ${SOURCES})
set_target_properties(BloodHorn-uefi PROPERTIES
    SUFFIX ".efi"
    COMPILE_DEFINITIONS "UEFI_BUILD"
    LINK_FLAGS "-nostdlib -Wl,--subsystem,10"
)

# Assembly files
set(ASM_SOURCES
    bios/bootsector.asm
    net/pxe_rm.asm
)

# Create BIOS boot sector
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/bootsector.bin
    COMMAND nasm -f bin bios/bootsector.asm -o ${CMAKE_BINARY_DIR}/bootsector.bin
    DEPENDS bios/bootsector.asm
    COMMENT "Building BIOS boot sector"
)

# Custom targets
add_custom_target(bios ALL DEPENDS ${CMAKE_BINARY_DIR}/bootsector.bin BloodHorn-bios)
add_custom_target(uefi ALL DEPENDS BloodHorn-uefi)
add_custom_target(iso ALL DEPENDS bios uefi)

# Installation
install(TARGETS BloodHorn-uefi
    RUNTIME DESTINATION EFI/BOOT
    RENAME BOOTX64.EFI
)

install(FILES ${CMAKE_BINARY_DIR}/bootsector.bin
    DESTINATION boot
)
