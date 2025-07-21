#ifndef _BOOT_MENU_H_
#define _BOOT_MENU_H_

#include <Uefi.h>

// Function to add a boot menu entry
EFI_STATUS
AddBootEntry(
    IN CONST CHAR16 *Name,
    IN EFI_STATUS (*BootFunction)(VOID)
);

// Function to display and handle the boot menu
EFI_STATUS
ShowBootMenu(VOID);

// Function to print text at specific coordinates with colors
VOID
EFIAPI
PrintXY(
    IN UINTN X,
    IN UINTN Y,
    IN UINT32 FgColor,
    IN UINT32 BgColor,
    IN CONST CHAR16 *Format,
    ...
);

#endif // _BOOT_MENU_H_
