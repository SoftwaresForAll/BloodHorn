#include <Uefi.h>
#include "compat.h"
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DevicePath.h>
#include "boot/menu.h"
#include "boot/theme.h"
#include "boot/localization.h"
#include "boot/mouse.h"
#include "boot/secure.h"
#include "fs/fat32.h"
#include "security/crypto.h"
#include "scripting/lua.h"
#include "recovery/shell.h"
#include "plugins/plugin.h"
#include "net/pxe.h"
#include "boot/Arch32/linux.h"
#include "boot/Arch32/limine.h"
#include "boot/Arch32/multiboot1.h"
#include "boot/Arch32/multiboot2.h"
#include "boot/Arch32/chainload.h"
#include "boot/Arch32/ia32.h"
#include "boot/Arch32/x86_64.h"
#include "boot/Arch32/aarch64.h"
#include "boot/Arch32/riscv64.h"
#include "boot/Arch32/loongarch64.h"
#include "boot/Arch32/BloodChain/bloodchain.h"
#include "config/config_ini.h"
#include "config/config_json.h"
#include "config/config_env.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct boot_config {
    char default_entry[64];
    int menu_timeout;
    char kernel[128];
    char initrd[128];
    char cmdline[256];
};

static int load_boot_config(struct boot_config* cfg) {
    struct boot_menu_entry entries[16];
    int n = parse_ini("bloodhorn.ini", entries, 16);
    if (n > 0) {
        for (int i = 0; i < n; ++i) {
            if (strcmp(entries[i].section, "boot") == 0) {
                if (strcmp(entries[i].name, "default") == 0) {
                    strncpy(cfg->default_entry, entries[i].path, sizeof(cfg->default_entry) - 1);
                    cfg->default_entry[sizeof(cfg->default_entry) - 1] = '\0';
                }
                if (strcmp(entries[i].name, "menu_timeout") == 0) {
                    cfg->menu_timeout = atoi(entries[i].path);
                }
            } else if (strcmp(entries[i].section, "linux") == 0) {
                if (strcmp(entries[i].name, "kernel") == 0) {
                    strncpy(cfg->kernel, entries[i].path, sizeof(cfg->kernel) - 1);
                    cfg->kernel[sizeof(cfg->kernel) - 1] = '\0';
                }
                if (strcmp(entries[i].name, "initrd") == 0) {
                    strncpy(cfg->initrd, entries[i].path, sizeof(cfg->initrd) - 1);
                    cfg->initrd[sizeof(cfg->initrd) - 1] = '\0';
                }
                if (strcmp(entries[i].name, "cmdline") == 0) {
                    strncpy(cfg->cmdline, entries[i].path, sizeof(cfg->cmdline) - 1);
                    cfg->cmdline[sizeof(cfg->cmdline) - 1] = '\0';
                }
            }
        }
        return 0;
    }

    struct config_json json_entries[32];
    FILE* f = fopen("bloodhorn.json", "r");
    if (f) {
        char buf[4096];
        size_t len = fread(buf, 1, sizeof(buf) - 1, f);
        buf[len] = '\0';
        fclose(f);
        int m = config_json_parse(buf, json_entries, 32);
        for (int i = 0; i < m; ++i) {
            if (strcmp(json_entries[i].key, "boot.default") == 0) {
                strncpy(cfg->default_entry, json_entries[i].value, sizeof(cfg->default_entry) - 1);
                cfg->default_entry[sizeof(cfg->default_entry) - 1] = '\0';
            }
            if (strcmp(json_entries[i].key, "boot.menu_timeout") == 0) {
                cfg->menu_timeout = atoi(json_entries[i].value);
            }
            if (strcmp(json_entries[i].key, "linux.kernel") == 0) {
                strncpy(cfg->kernel, json_entries[i].value, sizeof(cfg->kernel) - 1);
                cfg->kernel[sizeof(cfg->kernel) - 1] = '\0';
            }
            if (strcmp(json_entries[i].key, "linux.initrd") == 0) {
                strncpy(cfg->initrd, json_entries[i].value, sizeof(cfg->initrd) - 1);
                cfg->initrd[sizeof(cfg->initrd) - 1] = '\0';
            }
            if (strcmp(json_entries[i].key, "linux.cmdline") == 0) {
                strncpy(cfg->cmdline, json_entries[i].value, sizeof(cfg->cmdline) - 1);
                cfg->cmdline[sizeof(cfg->cmdline) - 1] = '\0';
            }
        }
        return 0;
    }

    // Fallback to environment variables
    char val[256];
    if (config_env_get("BLOODHORN_DEFAULT", val, sizeof(val)) == 0) {
        strncpy(cfg->default_entry, val, sizeof(cfg->default_entry) - 1);
        cfg->default_entry[sizeof(cfg->default_entry) - 1] = '\0';
    }
    if (config_env_get("BLOODHORN_MENU_TIMEOUT", val, sizeof(val)) == 0) {
        cfg->menu_timeout = atoi(val);
    }
    if (config_env_get("BLOODHORN_LINUX_KERNEL", val, sizeof(val)) == 0) {
        strncpy(cfg->kernel, val, sizeof(cfg->kernel) - 1);
        cfg->kernel[sizeof(cfg->kernel) - 1] = '\0';
    }
    if (config_env_get("BLOODHORN_LINUX_INITRD", val, sizeof(val)) == 0) {
        strncpy(cfg->initrd, val, sizeof(cfg->initrd) - 1);
        cfg->initrd[sizeof(cfg->initrd) - 1] = '\0';
    }
    if (config_env_get("BLOODHORN_LINUX_CMDLINE", val, sizeof(val)) == 0) {
        strncpy(cfg->cmdline, val, sizeof(cfg->cmdline) - 1);
        cfg->cmdline[sizeof(cfg->cmdline) - 1] = '\0';
    }

    return 0;
}

// Forward declarations for boot wrappers
EFI_STATUS boot_bloodchain_wrapper(void);
EFI_STATUS boot_linux_kernel_wrapper(void);
EFI_STATUS boot_multiboot2_kernel_wrapper(void);
EFI_STATUS boot_limine_kernel_wrapper(void);
EFI_STATUS boot_chainload_wrapper(void);
EFI_STATUS boot_pxe_network_wrapper(void);
EFI_STATUS boot_recovery_shell_wrapper(void);
EFI_STATUS boot_uefi_shell_wrapper(EFI_HANDLE ImageHandle);  // added parameter here
EFI_STATUS exit_to_firmware_wrapper(void);
EFI_STATUS boot_ia32_wrapper(void);
EFI_STATUS boot_x86_64_wrapper(void);
EFI_STATUS boot_aarch64_wrapper(void);
EFI_STATUS boot_riscv64_wrapper(void);
EFI_STATUS boot_loongarch64_wrapper(void);

// Helper to load theme and language from config files
static void LoadThemeAndLanguageFromConfig(void) {
    struct BootMenuTheme theme = {0};
    char lang[8] = "en";

    FILE* f = fopen("bloodhorn.ini", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "theme_background_color")) sscanf(line, "%*[^=]=%x", &theme.background_color);
            if (strstr(line, "theme_header_color")) sscanf(line, "%*[^=]=%x", &theme.header_color);
            if (strstr(line, "theme_highlight_color")) sscanf(line, "%*[^=]=%x", &theme.highlight_color);
            if (strstr(line, "theme_text_color")) sscanf(line, "%*[^=]=%x", &theme.text_color);
            if (strstr(line, "theme_selected_text_color")) sscanf(line, "%*[^=]=%x", &theme.selected_text_color);
            if (strstr(line, "theme_footer_color")) sscanf(line, "%*[^=]=%x", &theme.footer_color);
            if (strstr(line, "theme_background_image")) {
                char imgfile[128];
                sscanf(line, "%*[^=]=%127s", imgfile);
                theme.background_image = LoadImageFile(imgfile); // Assumes LoadImageFile defined elsewhere
            }
            if (strstr(line, "language")) sscanf(line, "%*[^=]=%7s", lang);
        }
        fclose(f);
    } else {
        f = fopen("bloodhorn.json", "r");
        if (f) {
            char json[4096];
            size_t len = fread(json, 1, sizeof(json) - 1, f);
            json[len] = '\0';
            fclose(f);

            struct config_json entries[64];
            int count = config_json_parse(json, entries, 64);
            for (int i = 0; i < count; ++i) {
                if (strcmp(entries[i].key, "theme.background_color") == 0) theme.background_color = (uint32_t)strtoul(entries[i].value, NULL, 16);
                if (strcmp(entries[i].key, "theme.header_color") == 0) theme.header_color = (uint32_t)strtoul(entries[i].value, NULL, 16);
                if (strcmp(entries[i].key, "theme.highlight_color") == 0) theme.highlight_color = (uint32_t)strtoul(entries[i].value, NULL, 16);
                if (strcmp(entries[i].key, "theme.text_color") == 0) theme.text_color = (uint32_t)strtoul(entries[i].value, NULL, 16);
                if (strcmp(entries[i].key, "theme.selected_text_color") == 0) theme.selected_text_color = (uint32_t)strtoul(entries[i].value, NULL, 16);
                if (strcmp(entries[i].key, "theme.footer_color") == 0) theme.footer_color = (uint32_t)strtoul(entries[i].value, NULL, 16);
                if (strcmp(entries[i].key, "theme.background_image") == 0) theme.background_image = LoadImageFile(entries[i].value);
                if (strcmp(entries[i].key, "language") == 0) strncpy(lang, entries[i].value, sizeof(lang) - 1);
            }
        }
    }

    SetBootMenuTheme(&theme); // Assumes defined elsewhere
    SetLanguage(lang);        // Assumes defined elsewhere
}

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput = NULL;

    gST = SystemTable;
    gBS = SystemTable->BootServices;
    gRT = SystemTable->RuntimeServices;

    Status = gBS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage);
    if (EFI_ERROR(Status)) return Status;

    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&GraphicsOutput);

    gST->ConOut->Reset(gST->ConOut, FALSE);
    gST->ConOut->SetMode(gST->ConOut, 0);
    gST->ConOut->ClearScreen(gST->ConOut);

    LoadThemeAndLanguageFromConfig();
    InitMouse();

    AddBootEntry(L"BloodChain Boot Protocol", boot_bloodchain_wrapper);
    AddBootEntry(L"Linux Kernel", boot_linux_kernel_wrapper);
    AddBootEntry(L"Multiboot2 Kernel", boot_multiboot2_kernel_wrapper);
    AddBootEntry(L"Limine Kernel", boot_limine_kernel_wrapper);
    AddBootEntry(L"Chainload Bootloader", boot_chainload_wrapper);
    AddBootEntry(L"PXE Network Boot", boot_pxe_network_wrapper);
    AddBootEntry(L"IA-32 (32-bit x86)", boot_ia32_wrapper);
    AddBootEntry(L"x86-64 (64-bit x86)", boot_x86_64_wrapper);
    AddBootEntry(L"ARM64 (aarch64)", boot_aarch64_wrapper);
    AddBootEntry(L"RISC-V 64", boot_riscv64_wrapper);
    AddBootEntry(L"LoongArch 64", boot_loongarch64_wrapper);
    AddBootEntry(L"Recovery Shell", boot_recovery_shell_wrapper);
    AddBootEntry(L"UEFI Shell", (EFI_STATUS (*)(void))boot_uefi_shell_wrapper);
    AddBootEntry(L"Exit to UEFI Firmware", exit_to_firmware_wrapper);

    Status = ShowBootMenu();
    if (Status == EFI_SUCCESS) {
        VOID* KernelBuffer = NULL;
        UINTN KernelSize = 0;
        Status = LoadAndVerifyKernel(L"kernel.efi", &KernelBuffer, &KernelSize);
        if (!EFI_ERROR(Status)) {
            Status = ExecuteKernel(KernelBuffer, KernelSize, NULL);
            if (!EFI_ERROR(Status)) return EFI_SUCCESS;
        }
    }

    Print(L"\r\n  No bootable device found or kernel failed.\r\n");
    Print(L"  Press any key to reboot...");
    EFI_INPUT_KEY Key;
    UINTN Index;
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Index);
    gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    gST->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    return EFI_DEVICE_ERROR;
}

// Boot wrapper implementations
EFI_STATUS boot_linux_kernel_wrapper(void) {
    return linux_load_kernel("/boot/vmlinuz", "/boot/initrd.img", "root=/dev/sda1 ro");
}

EFI_STATUS boot_multiboot2_kernel_wrapper(void) {
    return multiboot2_load_kernel("/boot/vmlinuz-mb2", "root=/dev/sda1 ro");
}

EFI_STATUS boot_limine_kernel_wrapper(void) {
    return limine_load_kernel("/boot/vmlinuz-limine", "root=/dev/sda1 ro");
}

EFI_STATUS boot_chainload_wrapper(void) {
    return chainload_file("/boot/grub2.bin");
}

EFI_STATUS boot_pxe_network_wrapper(void) {
    return pxe_boot_kernel("/boot/vmlinuz", "/boot/initrd.img", "root=/dev/sda1 ro");
}

EFI_STATUS boot_recovery_shell_wrapper(void) {
    return shell_start();
}

EFI_STATUS boot_uefi_shell_wrapper(EFI_HANDLE ImageHandle) {
    return gBS->StartImage(ImageHandle, NULL, NULL);
}

EFI_STATUS exit_to_firmware_wrapper(void) {
    gST->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}

EFI_STATUS boot_ia32_wrapper(void) {
    return ia32_load_kernel("/boot/vmlinuz-ia32", "/boot/initrd-ia32.img", "root=/dev/sda1 ro");
}

EFI_STATUS boot_x86_64_wrapper(void) {
    return x86_64_load_kernel("/boot/vmlinuz-x86_64", "/boot/initrd-x86_64.img", "root=/dev/sda1 ro");
}

EFI_STATUS boot_aarch64_wrapper(void) {
    return aarch64_load_kernel("/boot/Image-aarch64", "/boot/initrd-aarch64.img", "root=/dev/sda1 ro");
}

EFI_STATUS boot_riscv64_wrapper(void) {
    return riscv64_load_kernel("/boot/Image-riscv64", "/boot/initrd-riscv64.img", "root=/dev/sda1 ro");
}

EFI_STATUS boot_loongarch64_wrapper(void) {
    return loongarch64_load_kernel("/boot/Image-loongarch64", "/boot/initrd-loongarch64.img", "root=/dev/sda1 ro");
}

// BloodChain Boot Protocol implementation
EFI_STATUS boot_bloodchain_wrapper(void) {
    EFI_STATUS Status;
    EFI_PHYSICAL_ADDRESS KernelBase = 0x100000; // 1MB mark
    EFI_PHYSICAL_ADDRESS BcbpBase;
    struct bcbp_header *hdr;
    
    // Allocate memory for BCBP header (4KB aligned)
    Status = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 
                              EFI_SIZE_TO_PAGES(64 * 1024), &BcbpBase);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to allocate memory for BCBP header\n");
        return Status;
    }
    
    // Initialize BCBP header
    hdr = (struct bcbp_header *)(UINTN)BcbpBase;
    bcbp_init(hdr, KernelBase, 0);  // 0 for boot device (set by bootloader)
    
    // Load kernel
    const char *kernel_path = "kernel.elf";  // Default kernel path
    const char *initrd_path = "initrd.img";  // Default initrd path
    const char *cmdline = "root=/dev/sda1 ro"; // Default command line
    
    // Load kernel into memory
    EFI_PHYSICAL_ADDRESS KernelLoadAddr = KernelBase;
    UINTN KernelSize = 0;
    
    // Load kernel file
    Status = LoadFileToMemory(kernel_path, &KernelLoadAddr, &KernelSize);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to load kernel: %r\n", Status);
        return Status;
    }
    
    // Add kernel module
    bcbp_add_module(hdr, KernelLoadAddr, KernelSize, "kernel", 
                   BCBP_MODTYPE_KERNEL, cmdline);
    
    // Load initrd if it exists
    EFI_PHYSICAL_ADDRESS InitrdLoadAddr = KernelLoadAddr + ALIGN_UP(KernelSize, 0x1000);
    UINTN InitrdSize = 0;
    
    if (FileExists(initrd_path)) {
        Status = LoadFileToMemory(initrd_path, &InitrdLoadAddr, &InitrdSize);
        if (!EFI_ERROR(Status) && InitrdSize > 0) {
            bcbp_add_module(hdr, InitrdLoadAddr, InitrdSize, "initrd", 
                          BCBP_MODTYPE_INITRD, NULL);
        }
    }
    
    // Set up ACPI and SMBIOS if available
    EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *Rsdp = NULL;
    EFI_CONFIGURATION_TABLE *ConfigTable = gST->ConfigurationTable;
    
    for (UINTN i = 0; i < gST->NumberOfTableEntries; i++) {
        if (CompareGuid(&ConfigTable[i].VendorGuid, &gEfiAcpi20TableGuid)) {
            Rsdp = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)ConfigTable[i].VendorTable;
            break;
        }
    }
    
    if (Rsdp) {
        bcbp_set_acpi_rsdp(hdr, (UINT64)(UINTN)Rsdp);
    }
    
    // Find SMBIOS table
    for (UINTN i = 0; i < gST->NumberOfTableEntries; i++) {
        if (CompareGuid(&ConfigTable[i].VendorGuid, &gEfiSmbiosTableGuid)) {
            bcbp_set_smbios(hdr, (UINT64)(UINTN)ConfigTable[i].VendorTable);
            break;
        }
    }
    
    // Set framebuffer information if available
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop = NULL;
    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (void **)&Gop);
    if (!EFI_ERROR(Status) && Gop) {
        bcbp_set_framebuffer(hdr, Gop->Mode->FrameBufferBase);
    }
    
    // Set secure boot status
    UINT8 SecureBoot = 0;
    UINTN DataSize = sizeof(SecureBoot);
    if (EFI_ERROR(gRT->GetVariable(L"SecureBoot", &gEfiGlobalVariableGuid, 
                                 NULL, &DataSize, &SecureBoot)) == EFI_SUCCESS) {
        hdr->secure_boot = SecureBoot;
    }
    
    // Set UEFI 64-bit flag
    hdr->uefi_64bit = (sizeof(UINTN) == 8) ? 1 : 0;
    
    // Validate BCBP structure
    if (bcbp_validate(hdr) != 0) {
        Print(L"Invalid BCBP structure\n");
        return EFI_LOAD_ERROR;
    }
    
    // Print boot information
    Print(L"Booting with BloodChain Boot Protocol\n");
    Print(L"  Kernel: 0x%llx (%u bytes)\n", KernelLoadAddr, KernelSize);
    if (InitrdSize > 0) {
        Print(L"  Initrd: 0x%llx (%u bytes)\n", InitrdLoadAddr, InitrdSize);
    }
    Print(L"  Command line: %a\n", cmdline);
    
    // Jump to kernel
    typedef void (*KernelEntry)(struct bcbp_header *);
    KernelEntry EntryPoint = (KernelEntry)(UINTN)KernelLoadAddr;
    
    // Disable interrupts and jump to kernel
    gBS->ExitBootServices(gImageHandle, 0);
    EntryPoint(hdr);
    
    // We should never get here
    return EFI_LOAD_ERROR;
}
