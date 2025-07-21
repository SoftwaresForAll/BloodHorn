#include <Uefi.h>
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
#include "config/config_ini.h"
#include "config/config_json.h"
#include "config/config_env.h"

struct boot_config {
    char default_entry[64];
    int menu_timeout;
    char kernel[128];
    char initrd[128];
    char cmdline[256];
};

static int load_boot_config(struct boot_config* cfg) {
    // Try INI first
    struct boot_menu_entry entries[16];
    int n = parse_ini("bloodhorn.ini", entries, 16);
    if (n > 0) {
        for (int i = 0; i < n; ++i) {
            if (strcmp(entries[i].section, "boot") == 0) {
                if (strcmp(entries[i].name, "default") == 0) strncpy(cfg->default_entry, entries[i].path, 63);
                if (strcmp(entries[i].name, "menu_timeout") == 0) cfg->menu_timeout = atoi(entries[i].path);
            } else if (strcmp(entries[i].section, "linux") == 0) {
                if (strcmp(entries[i].name, "kernel") == 0) strncpy(cfg->kernel, entries[i].path, 127);
                if (strcmp(entries[i].name, "initrd") == 0) strncpy(cfg->initrd, entries[i].path, 127);
                if (strcmp(entries[i].name, "cmdline") == 0) strncpy(cfg->cmdline, entries[i].path, 255);
            }
        }
        return 0;
    }
    // Try JSON next
    struct config_json json_entries[32];
    FILE* f = fopen("bloodhorn.json", "r");
    if (f) {
        char buf[4096];
        size_t len = fread(buf, 1, sizeof(buf)-1, f);
        buf[len] = 0;
        fclose(f);
        int m = config_json_parse(buf, json_entries, 32);
        for (int i = 0; i < m; ++i) {
            if (strcmp(json_entries[i].key, "boot.default") == 0) strncpy(cfg->default_entry, json_entries[i].value, 63);
            if (strcmp(json_entries[i].key, "boot.menu_timeout") == 0) cfg->menu_timeout = atoi(json_entries[i].value);
            if (strcmp(json_entries[i].key, "linux.kernel") == 0) strncpy(cfg->kernel, json_entries[i].value, 127);
            if (strcmp(json_entries[i].key, "linux.initrd") == 0) strncpy(cfg->initrd, json_entries[i].value, 127);
            if (strcmp(json_entries[i].key, "linux.cmdline") == 0) strncpy(cfg->cmdline, json_entries[i].value, 255);
        }
        return 0;
    }
    // Fallback to environment
    char val[256];
    if (config_env_get("BLOODHORN_DEFAULT", val, sizeof(val)) == 0) strncpy(cfg->default_entry, val, 63);
    if (config_env_get("BLOODHORN_MENU_TIMEOUT", val, sizeof(val)) == 0) cfg->menu_timeout = atoi(val);
    if (config_env_get("BLOODHORN_LINUX_KERNEL", val, sizeof(val)) == 0) strncpy(cfg->kernel, val, 127);
    if (config_env_get("BLOODHORN_LINUX_INITRD", val, sizeof(val)) == 0) strncpy(cfg->initrd, val, 127);
    if (config_env_get("BLOODHORN_LINUX_CMDLINE", val, sizeof(val)) == 0) strncpy(cfg->cmdline, val, 255);
    return 0;
}

// Function declarations for boot wrappers
EFI_STATUS boot_linux_kernel_wrapper(void);
EFI_STATUS boot_multiboot2_kernel_wrapper(void);
EFI_STATUS boot_limine_kernel_wrapper(void);
EFI_STATUS boot_chainload_wrapper(void);
EFI_STATUS boot_pxe_network_wrapper(void);
EFI_STATUS boot_recovery_shell_wrapper(void);
EFI_STATUS boot_uefi_shell_wrapper(void);
EFI_STATUS exit_to_firmware_wrapper(void);
EFI_STATUS boot_ia32_wrapper(void);
EFI_STATUS boot_x86_64_wrapper(void);
EFI_STATUS boot_aarch64_wrapper(void);
EFI_STATUS boot_riscv64_wrapper(void);
EFI_STATUS boot_loongarch64_wrapper(void);

// Helper: load theme and language from config 
static void LoadThemeAndLanguageFromConfig() {
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
                sscanf(line, "%*[^=]=%s", imgfile);
                theme.background_image = LoadImageFile(imgfile);
            }
            if (strstr(line, "language")) sscanf(line, "%*[^=]=%7s", lang);
        }
        fclose(f);
    } else {
        f = fopen("bloodhorn.json", "r");
        if (f) {
            char json[4096];
            size_t len = fread(json, 1, sizeof(json)-1, f);
            json[len] = 0;
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
                if (strcmp(entries[i].key, "language") == 0) strncpy(lang, entries[i].value, 7);
            }
            fclose(f);
        }
    }
    SetBootMenuTheme(&theme);
    SetLanguage(lang);
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

    // Load theme and language from config
    LoadThemeAndLanguageFromConfig();
    InitMouse();

    // Add boot menu entries with full architecture support
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
    AddBootEntry(L"UEFI Shell", boot_uefi_shell_wrapper);
    AddBootEntry(L"Exit to UEFI Firmware", exit_to_firmware_wrapper);

    // Show the graphical boot menu
    Status = ShowBootMenu();
    if (Status == EFI_SUCCESS) {
        // Here, load and verify the selected kernel (example: kernel.efi)
        VOID* KernelBuffer = NULL;
        UINTN KernelSize = 0;
        Status = LoadAndVerifyKernel(L"kernel.efi", &KernelBuffer, &KernelSize);
        if (!EFI_ERROR(Status)) {
            Status = ExecuteKernel(KernelBuffer, KernelSize, NULL);
            if (!EFI_ERROR(Status)) return EFI_SUCCESS;
        }
    }

    // If we get here, no bootable device was found or kernel failed.
    Print(L"\r\n  No bootable device found or kernel failed.\r\n");
    Print(L"  Press any key to reboot...");
    EFI_INPUT_KEY Key;
    UINTN Index;
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Index);
    gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    gST->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    return EFI_DEVICE_ERROR;
}

// Boot wrapper functions for menu integration
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

EFI_STATUS boot_uefi_shell_wrapper(void) {
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
