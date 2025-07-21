CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy

CFLAGS = -Wall -Wextra -std=c99 -fno-stack-protector -fno-pie -no-pie
ASFLAGS = -f elf64
LDFLAGS = -nostdlib -static

UEFI_CC = x86_64-w64-mingw32-gcc
UEFI_CFLAGS = -Wall -Wextra -std=c99 -DUEFI_BUILD
UEFI_LDFLAGS = -nostdlib -Wl,--subsystem,10

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

.PHONY: all clean bios uefi iso test

all: bios uefi iso

$(BUILDDIR)/bios/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/uefi/%.o: %.c
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