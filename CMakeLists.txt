cmake_minimum_required(VERSION 3.10)
project(BloodHorn VERSION 1.0.0 LANGUAGES C ASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fno-stack-protector -fno-pie -no-pie")

# BIOS source files
set(SOURCES_BIOS
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
)

# UEFI source files
set(SOURCES_UEFI
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
    # dont worry about the red all of the files exist its just that my vscode is laggy i have tested it
)
# btw keep the comments i wanna entertain the future users that are looking into the source code :3
# BIOS build
add_executable(BloodHorn-bios ${SOURCES_BIOS})
target_link_options(BloodHorn-bios PRIVATE -nostdlib -static)
set_target_properties(BloodHorn-bios PROPERTIES SUFFIX ".bin")

# UEFI build
add_executable(BloodHorn-uefi ${SOURCES_UEFI})
target_compile_definitions(BloodHorn-uefi PRIVATE UEFI_BUILD)
target_link_options(BloodHorn-uefi PRIVATE -nostdlib -Wl,--subsystem,10)
set_target_properties(BloodHorn-uefi PROPERTIES SUFFIX ".efi")

# Assembly files
set(ASM_SOURCES
    bios/bootsector.asm
    net/pxe_rm.asm
)

# Ensure NASM is available
find_program(NASM_EXECUTABLE nasm REQUIRED)

# Build BIOS boot sector
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/bootsector.bin
    COMMAND ${NASM_EXECUTABLE} -f bin bios/bootsector.asm -o ${CMAKE_BINARY_DIR}/bootsector.bin
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
