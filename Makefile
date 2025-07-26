# Detect OS
UNAME_S := $(shell uname -s)

# Compiler and flags
CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy

CFLAGS = -Wall -Wextra -std=c99 -fno-stack-protector -fno-pie -no-pie
ASFLAGS = -f elf64
LDFLAGS = -nostdlib -static

# UEFI settings
UEFI_CC = x86_64-w64-mingw32-gcc
UEFI_CFLAGS = -Wall -Wextra -std=c99 -DUEFI_BUILD
UEFI_LDFLAGS = -nostdlib -Wl,--subsystem,10

# Detect and set UEFI paths
ifeq ($(UNAME_S),Linux)
    UEFI_INC_PATHS = /usr/include/efi /usr/include/efi/x86_64
    UEFI_LIB_PATHS = /usr/lib
    UEFI_CFLAGS += $(addprefix -I,$(UEFI_INC_PATHS))
    UEFI_LDFLAGS += $(addprefix -L,$(UEFI_LIB_PATHS))
    UEFI_CRT_OBJS = $(shell find /usr/lib* -name 'crt0-efi-x86_64.o' 2>/dev/null | head -1)
    UEFI_LDS = $(shell find /usr/lib* -name 'elf_x86_64_efi.lds' 2>/dev/null | head -1)
else ifeq ($(OS),Windows_NT)
    UEFI_INC_PATHS = /mingw64/include/efi /mingw64/include/efi/protocol
    UEFI_LIB_PATHS = /mingw64/lib
    UEFI_CFLAGS += $(addprefix -I,$(UEFI_INC_PATHS))
    UEFI_LDFLAGS += $(addprefix -L,$(UEFI_LIB_PATHS))
    UEFI_CRT_OBJS = /mingw64/lib/crt0-x86_64.o
    UEFI_LDS = /mingw64/x86_64-w64-mingw32/lib/efi.lds
endif

# Check for UEFI dependencies
UEFI_DEPS = $(UEFI_CC) $(UEFI_CRT_OBJS) $(UEFI_LDS)

# Function to check if a command exists
CHECK_CMD = $(shell command -v $(1) 2> /dev/null)

# Function to install UEFI deps on Linux
install_uefi_deps_linux:
	@echo "Installing UEFI development dependencies..."
	if [ -f /etc/debian_version ]; then \
		sudo apt-get update && sudo apt-get install -y gnu-efi; \
	elif [ -f /etc/redhat-release ]; then \
		sudo dnf install -y gnu-efi-devel; \
	elif [ -f /etc/arch-release ]; then \
		sudo pacman -S --noconfirm gnu-efi; \
	else \
		echo "Unsupported Linux distribution. Please install gnu-efi manually."; \
		exit 1; \
	fi

# Function to install UEFI deps on Windows
install_uefi_deps_win:
	@echo "Installing UEFI development dependencies..."
	if [ ! -d "/mingw64" ]; then \
		echo "MSYS2 not found. Please install MSYS2 from https://www.msys2.org/"; \
		exit 1; \
	fi
	pacman -S --noconfirm --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-gnu-efi

# Check if UEFI deps are installed
UEFI_DEPS_INSTALLED = $(shell (which $(UEFI_CC) >/dev/null 2>&1 && echo 1) || echo 0)

# Check and install UEFI dependencies if needed
check_uefi_deps:
	@echo "Checking UEFI toolchain..."
	@if [ "$(UEFI_DEPS_INSTALLED)" = "0" ]; then \
		echo "UEFI toolchain not found. Installing..."; \
		$(MAKE) install_uefi_deps_$(if $(filter Windows%,$(OS)),win,linux) || (echo "Failed to install UEFI dependencies"; exit 1); \
	else \
		echo "UEFI toolchain is already installed."; \
	fi
	@echo "Verifying UEFI components..."
	@missing_deps=0; \
	for dep in $(UEFI_DEPS); do \
		if [ ! -e "$$dep" ]; then \
			echo "Missing UEFI component: $$dep"; \
			missing_deps=1; \
		fi; \
	done; \
	if [ "$$missing_deps" = "1" ]; then \
		echo "Some UEFI components are missing. Attempting to fix..."; \
		$(MAKE) install_uefi_deps_$(if $(filter Windows%,$(OS)),win,linux) || (echo "Failed to install missing UEFI components"; exit 1); \
	fi

BUILDDIR = build
ISODIR = iso

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

BIOS_OBJECTS = $(SOURCES:%.c=$(BUILDDIR)/bios/%.o)
UEFI_OBJECTS = $(SOURCES:%.c=$(BUILDDIR)/uefi/%.o)

.PHONY: all clean bios uefi iso test check_uefi_deps install_uefi_deps_linux install_uefi_deps_win

# Main build targets
all: bios uefi iso

# Dependencies for UEFI build
uefi: check_uefi_deps $(BUILDDIR)/BloodHorn.efi

$(BUILDDIR)/bios/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/uefi/%.o: %.c | check_uefi_deps
	@mkdir -p $(dir $@)
	$(UEFI_CC) $(UEFI_CFLAGS) -c $< -o $@

$(BUILDDIR)/bios/bootsector.bin: bios/bootsector.asm
	@mkdir -p $(dir $@)
	$(AS) -f bin $< -o $@

$(BUILDDIR)/bios/stage2.bin: $(BIOS_OBJECTS)
	$(LD) $(LDFLAGS) -T bios.ld -o $@ $^
	$(OBJCOPY) -O binary $@ $@

bios: $(BUILDDIR)/bios/bootsector.bin $(BUILDDIR)/bios/stage2.bin

uefi: $(BUILDDIR)/BloodHorn.efi

$(BUILDDIR)/BloodHorn.efi: $(UEFI_OBJECTS)
	$(UEFI_CC) $(UEFI_LDFLAGS) -o $@ $^

iso: $(ISODIR)/BloodHorn.iso

$(ISODIR)/BloodHorn.iso: bios uefi
	@mkdir -p $(ISODIR)/EFI/BOOT
	@mkdir -p $(ISODIR)/boot
	cp $(BUILDDIR)/BloodHorn.efi $(ISODIR)/EFI/BOOT/BOOTX64.EFI
	cp $(BUILDDIR)/bios/bootsector.bin $(ISODIR)/boot/
	cp $(BUILDDIR)/bios/stage2.bin $(ISODIR)/boot/
	genisoimage -o $@ -b boot/bootsector.bin -no-emul-boot $(ISODIR)

test: iso
	qemu-system-x86_64 -cdrom $(ISODIR)/BloodHorn.iso -m 512

test-bios: bios
	qemu-system-x86_64 -drive file=$(BUILDDIR)/bios/stage2.bin,format=raw -m 512

test-uefi: uefi
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -drive file=$(BUILDDIR)/BloodHorn.efi,format=raw -m 512

clean:
	rm -rf $(BUILDDIR) $(ISODIR) 