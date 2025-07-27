# Toolchain
CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy

# Build directories
BUILDDIR = build
ISODIR = iso

# UEFI toolchain
UEFI_CC = x86_64-w64-mingw32-gcc
UEFI_OBJCOPY = x86_64-w64-mingw32-objcopy
UEFI_TARGET = x86_64-w64-mingw32

# UEFI flags
UEFI_CFLAGS = -I/usr/include/efi -I/usr/include/efi/x86_64 -I/usr/include/efi/protocol \
              -DEFI_FUNCTION_WRAPPER -fno-stack-protector -fpic -fshort-wchar -mno-red-zone \
              -Wall -Wextra -std=c99 -DARCH_UEFI

UEFI_LDFLAGS = -nostdlib -Wl,-dll,-shared -Wl,--subsystem,10 -e efi_main \
               -L/usr/lib -lgnuefi -lefi

# Source files
SOURCES = main.c
SOURCES += $(wildcard uefi/*.c)
SOURCES += $(wildcard fs/*.c)
SOURCES += $(wildcard net/*.c)
SOURCES += $(wildcard security/*.c)
SOURCES += $(wildcard config/*.c)

# Object files
OBJECTS = $(addprefix $(BUILDDIR)/,$(SOURCES:.c=.o))

# Default target
.PHONY: all
all: uefi

# UEFI build
uefi: $(BUILDDIR)/BloodHorn.efi

# Rule to compile UEFI files
$(BUILDDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(UEFI_CC) $(UEFI_CFLAGS) -c $< -o $@

# Link UEFI application
$(BUILDDIR)/BloodHorn.efi: $(OBJECTS)
	$(UEFI_CC) -o $(BUILDDIR)/BloodHorn.so $(OBJECTS) $(UEFI_LDFLAGS)
	$(UEFI_OBJCOPY) --target=efi-app-x86_64 $(BUILDDIR)/BloodHorn.so $@

# Create a bootable UEFI ISO
iso: uefi
	@mkdir -p $(ISODIR)/EFI/BOOT
	cp $(BUILDDIR)/BloodHorn.efi $(ISODIR)/EFI/BOOT/BOOTX64.EFI
	xorriso -as mkisofs -e EFI/BOOT/BOOTX64.EFI -no-emul-boot -o BloodHorn.iso $(ISODIR)

# Run in QEMU for testing
run: iso
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom BloodHorn.iso -m 512 -vga std

debug: uefi
	qemu-system-x86_64 -s -S -bios /usr/share/ovmf/OVMF.fd -hda fat:rw:$(BUILDDIR) -m 512 -vga std &
	gdb -ex "target remote localhost:1234" -ex "symbol-file $(BUILDDIR)/BloodHorn.so"

# Setup and info
uefi-setup:
	@echo "UEFI Development Setup Instructions:"
	@echo "1. Install required packages:"
	@echo "   sudo apt-get install gcc gcc-mingw-w64-x86-64 binutils-mingw-w64-x86-64 gnu-efi ovmf"
	@echo "2. Build with 'make'"
	@echo "3. Test with 'make run' or 'make debug'"

# Clean target
uefi-info:
	@echo "UEFI Toolchain:"
	@echo "  UEFI_CC: $(UEFI_CC) ($(shell which $(UEFI_CC) || echo 'not found'))"
	@echo "  CRT Object: $(UEFI_CRT_OBJS)"
	@echo "  Linker Script: $(UEFI_LDS)"

# Cleanup
clean:
	rm -rf $(BUILDDIR) $(ISODIR)

.PHONY: all bios uefi iso test test-bios test-uefi clean uefi-setup uefi-info