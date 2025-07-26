# Toolchain
CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy

# Build directories
BUILDDIR = build
ISODIR = iso

# Compiler flags
CFLAGS = -Wall -Wextra -std=c99 -fno-stack-protector -fno-pie -no-pie
ASFLAGS = -f bin
LDFLAGS = -nostdlib -static -T bios.ld

# UEFI toolchain
UEFI_CC = x86_64-w64-mingw32-gcc
UEFI_CFLAGS = -Wall -Wextra -std=c99 -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -DEFI_FUNCTION_WRAPPER -DARCH_$(ARCH)
UEFI_LDFLAGS = -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main

# Source files
SOURCES = main.c
SOURCES += $(wildcard boot/*.c)
SOURCES += $(wildcard boot/Arch32/*.c)
SOURCES += $(wildcard uefi/*.c)
SOURCES += $(wildcard fs/*.c)
SOURCES += $(wildcard net/*.c)
SOURCES += $(wildcard security/*.c)
SOURCES += $(wildcard scripting/*.c)
SOURCES += $(wildcard recovery/*.c)
SOURCES += $(wildcard plugins/*.c)
SOURCES += $(wildcard config/*.c)
SOURCES += $(wildcard multiboot/*.c)

# Object files
BIOS_OBJECTS = $(addprefix $(BUILDDIR)/bios/,$(SOURCES:.c=.o))
UEFI_OBJECTS = $(addprefix $(BUILDDIR)/uefi/,$(SOURCES:.c=.o))

# Default target
.PHONY: all
all: bios uefi

# BIOS build
bios: $(BUILDDIR)/bios/bootsector.bin $(BUILDDIR)/bios/stage2.bin

$(BUILDDIR)/bios/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/bios/bootsector.bin: bios/bootsector.asm
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILDDIR)/bios/stage2.bin: $(BIOS_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^
	$(OBJCOPY) -O binary $@ $@

# UEFI build
UEFI_CRT_OBJS = $(shell find /usr /mingw64 -name 'crt0-efi-x86_64.o' 2>/dev/null | head -1)
UEFI_LDS = $(shell find /usr /mingw64 -name 'elf_x86_64_efi.lds' 2>/dev/null | head -1)

uefi: $(BUILDDIR)/BloodHorn.efi

$(BUILDDIR)/uefi/%.o: %.c
	@mkdir -p $(@D)
	$(UEFI_CC) $(UEFI_CFLAGS) -c $< -o $@

$(BUILDDIR)/BloodHorn.efi: $(UEFI_OBJECTS)
	@if [ -z "$(UEFI_CRT_OBJS)" ] || [ ! -f "$(UEFI_CRT_OBJS)" ]; then \
		echo "Error: UEFI CRT object file not found."; \
		echo "Run 'make uefi-setup' for setup instructions."; \
		exit 1; \
	fi
	$(UEFI_CC) $(UEFI_LDFLAGS) -o $@ $^ $(UEFI_CRT_OBJS) -lefi -lgnuefi

# ISO generation
iso: $(ISODIR)/BloodHorn.iso

$(ISODIR)/BloodHorn.iso: bios uefi
	@mkdir -p $(ISODIR)/EFI/BOOT
	@mkdir -p $(ISODIR)/boot
	cp $(BUILDDIR)/BloodHorn.efi $(ISODIR)/EFI/BOOT/BOOTX64.EFI
	cp $(BUILDDIR)/bios/bootsector.bin $(ISODIR)/boot/
	cp $(BUILDDIR)/bios/stage2.bin $(ISODIR)/boot/
	xorriso -as mkisofs -b boot/bootsector.bin -no-emul-boot -o $@ $(ISODIR)

# Test targets
test: iso
	qemu-system-x86_64 -cdrom $(ISODIR)/BloodHorn.iso -m 512

test-bios: bios
	qemu-system-x86_64 -drive file=$(BUILDDIR)/bios/stage2.bin,format=raw -m 512

test-uefi: uefi
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -drive format=raw,file=fat:rw:$(BUILDDIR) -m 512

# Setup and info
uefi-setup:
	@echo "UEFI Development Setup Instructions:"
	@echo "1. For Linux (Debian/Ubuntu):"
	@echo "   sudo apt-get install gnu-efi"
	@echo ""
	@echo "2. For Linux (Fedora/RHEL):"
	@echo "   sudo dnf install gnu-efi-devel"
	@echo ""
	@echo "3. For Windows (MSYS2):"
	@echo "   pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-gnu-efi"
	@echo ""
	@echo "After installation, run 'make uefi-info' to verify the setup"

uefi-info:
	@echo "UEFI Toolchain:"
	@echo "  UEFI_CC: $(UEFI_CC) ($(shell which $(UEFI_CC) || echo 'not found'))"
	@echo "  CRT Object: $(UEFI_CRT_OBJS)"
	@echo "  Linker Script: $(UEFI_LDS)"

# Cleanup
clean:
	rm -rf $(BUILDDIR) $(ISODIR)

.PHONY: all bios uefi iso test test-bios test-uefi clean uefi-setup uefi-info