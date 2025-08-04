/*
 * BloodHorn Bootloader
 *
 * This file is part of BloodHorn and is licensed under the MIT License.
 * See the root of the repository for license details.
 */
#ifndef _BOOT_MENU_H_
#define _BOOT_MENU_H_

#include <Uefi.h>
#include "compat.h"

// Function to add a boot menu entry
EFI_STATUS EFIAPI
AddBootEntry(
    IN CONST CHAR16 *Name,
    IN EFI_STATUS (EFIAPI *BootFunction)(VOID)
);

// Function to display and handle the boot menu
EFI_STATUS EFIAPI
ShowBootMenu(VOID);

// Function to print text at specific coordinates with colors
VOID EFIAPI
PrintXY(
    IN UINTN X,
    IN UINTN Y,
    IN UINT32 FgColor,
    IN UINT32 BgColor,
    IN CONST CHAR16 *Format,
    ...
);

#endif // _BOOT_MENU_H_