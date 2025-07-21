#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleTextInEx.h>
#include "../uefi/graphics.h"
#include "theme.h"
#include "localization.h"
#include "mouse.h"

#define MAX_BOOT_ENTRIES 10
#define MAX_ENTRY_LENGTH 64

// Boot menu entry structure
typedef struct {
    CHAR16 Name[MAX_ENTRY_LENGTH];
    EFI_STATUS (*BootFunction)(VOID);
} BOOT_MENU_ENTRY;

// Global boot menu state
static BOOT_MENU_ENTRY BootEntries[MAX_BOOT_ENTRIES];
static UINTN BootEntryCount = 0;
static INTN SelectedEntry = 0;

/**
  Adds a new boot menu entry.
  
  @param[in] Name           The display name of the boot entry.
  @param[in] BootFunction   The function to call when this entry is selected.
  
  @retval EFI_SUCCESS       The entry was added successfully.
  @retval EFI_OUT_OF_RESOURCES  No more space is available for boot entries.
**/
EFI_STATUS
AddBootEntry(
    IN CONST CHAR16 *Name,
    IN EFI_STATUS (*BootFunction)(VOID)
) {
    if (BootEntryCount >= MAX_BOOT_ENTRIES) {
        return EFI_OUT_OF_RESOURCES;
    }
    
    // Copy the name and boot function
    StrnCpyS(
        BootEntries[BootEntryCount].Name,
        MAX_ENTRY_LENGTH,
        Name,
        MAX_ENTRY_LENGTH - 1
    );
    
    BootEntries[BootEntryCount].BootFunction = BootFunction;
    BootEntryCount++;
    
    return EFI_SUCCESS;
}

/**
  Draws the boot menu on the screen.
**/
VOID
DrawBootMenu() {
    // Clear the screen with a dark background
    ClearScreen(0x1A1A2E);
    
    // Draw title
    INT32 ScreenWidth = GraphicsOutput->Mode->Info->HorizontalResolution;
    INT32 ScreenHeight = GraphicsOutput->Mode->Info->VerticalResolution;
    
    // Draw header
    DrawRect(0, 0, ScreenWidth, 60, 0x2D2D4F);
    
    // Draw menu background
    INT32 MenuX = ScreenWidth / 4;
    INT32 MenuY = 100;
    INT32 MenuWidth = ScreenWidth / 2;
    INT32 MenuHeight = (BootEntryCount * 50) + 40;
    
    DrawRect(MenuX, MenuY, MenuWidth, MenuHeight, 0x2D2D4F);
    
    // Draw menu title
    INT32 TextX = MenuX + 20;
    INT32 TextY = MenuY + 20;
    
    // Draw menu items
    for (UINTN i = 0; i < BootEntryCount; i++) {
        // Highlight the selected entry
        if (i == (UINTN)SelectedEntry) {
            DrawRect(
                MenuX + 10,
                TextY - 5,
                MenuWidth - 20,
                40,
                0x4A4A8A
            );
        }
        
        // Draw the entry text
        PrintXY(
            TextX,
            TextY,
            (i == (UINTN)SelectedEntry) ? 0xFFFFFF : 0xCCCCCC,
            0x00000000,
            L"%s",
            BootEntries[i].Name
        );
        
        TextY += 50;
    }
    
    // Draw footer with instructions
    CHAR16 *Instructions = L"↑/↓: Select  Enter: Boot  ESC: Exit";
    UINTN InstructionsWidth = StrLen(Instructions) * 10; // Approximate width
    PrintXY(
        (ScreenWidth - InstructionsWidth) / 2,
        MenuY + MenuHeight + 20,
        0x8888AA,
        0x00000000,
        L"%s",
        Instructions
    );
}

/**
  Displays the boot menu and handles user input.
  
  @retval EFI_SUCCESS   A boot option was selected successfully.
  @retval Other         An error occurred or the user exited the menu.
**/
EFI_STATUS
ShowBootMenu() {
    EFI_STATUS Status;
    EFI_EVENT WaitList[1];
    UINTN WaitIndex;
    EFI_INPUT_KEY Key;
    
    // Initialize graphics if available
    Status = InitializeGraphics();
    if (EFI_ERROR(Status)) {
        // Fall back to text mode
        gST->ConOut->ClearScreen(gST->ConOut);
    }
    
    // Add default boot entries if none were added
    if (BootEntryCount == 0) {
        AddBootEntry(L"Exit to UEFI Firmware", NULL);
    }
    
    // Use theme for all colors
    const struct BootMenuTheme* theme = GetBootMenuTheme();

    // In DrawBootMenu, use theme->background_color, etc. for all DrawRect/PrintXY calls
    // If theme->background_image != NULL, draw it as the background
    // Add scrollable menu logic: if BootEntryCount > 10, only show 10 at a time, with up/down arrows
    // Add mouse support: highlight entry under mouse, select on click
    // Add localization: use GetLocalizedString for all menu text
    // Add hotkey support: allow user to press a key to jump to a menu entry
    
    // Main menu loop
    while (TRUE) {
        // Draw the menu
        if (!EFI_ERROR(Status)) {
            DrawBootMenu();
        } else {
            // Text mode fallback
            gST->ConOut->ClearScreen(gST->ConOut);
            Print(L"\r\n  BloodHorn Boot Menu\r\n\r\n");
            
            for (UINTN i = 0; i < BootEntryCount; i++) {
                Print(L"  %c %s\r\n", 
                    (i == (UINTN)SelectedEntry) ? '>' : ' ', 
                    BootEntries[i].Name);
            }
            
            Print(L"\r\n  Use arrow keys to select, Enter to boot, ESC to exit");
        }
        
        // Wait for key press
        WaitList[0] = gST->ConIn->WaitForKey;
        Status = gBS->WaitForEvent(1, WaitList, &WaitIndex);
        if (EFI_ERROR(Status)) {
            continue;
        }
        
        // Read the key
        Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
        if (EFI_ERROR(Status)) {
            continue;
        }
        
        // Handle key press
        switch (Key.UnicodeChar) {
            case CHAR_CARRIAGE_RETURN:
                // Enter key - boot the selected entry
                if (BootEntries[SelectedEntry].BootFunction != NULL) {
                    return BootEntries[SelectedEntry].BootFunction();
                }
                return EFI_SUCCESS;
                
            case 0:
                // Special key
                switch (Key.ScanCode) {
                    case SCAN_UP:
                        if (SelectedEntry > 0) {
                            SelectedEntry--;
                        } else {
                            SelectedEntry = BootEntryCount - 1;
                        }
                        break;
                        
                    case SCAN_DOWN:
                        if ((UINTN)SelectedEntry < BootEntryCount - 1) {
                            SelectedEntry++;
                        } else {
                            SelectedEntry = 0;
                        }
                        break;
                        
                    case SCAN_ESC:
                        return EFI_ABORTED;
                }
                break;
        }
    }
    
    return EFI_SUCCESS;
}

/**
  Helper function to print text at specific coordinates with foreground and background colors.
  
  @param[in] X          The X coordinate.
  @param[in] Y          The Y coordinate.
  @param[in] FgColor    The foreground color in 0xRRGGBB format.
  @param[in] BgColor    The background color in 0xRRGGBB format.
  @param[in] Format     The format string.
  @param[in] ...        Variable arguments for the format string.
**/
VOID
EFIAPI
PrintXY(
    IN UINTN X,
    IN UINTN Y,
    IN UINT32 FgColor,
    IN UINT32 BgColor,
    IN CONST CHAR16 *Format,
    ...
) {
    VA_LIST Args;
    CHAR16 Buffer[256];
    UINTN StrSize;
    
    // Format the string
    VA_START(Args, Format);
    UnicodeVSPrint(Buffer, sizeof(Buffer), Format, Args);
    VA_END(Args);
    
    // Set cursor position
    gST->ConOut->SetCursorPosition(gST->ConOut, X, Y);
    
    // Set text attributes
    gST->ConOut->SetAttribute(
        gST->ConOut,
        EFI_TEXT_ATTR(
            (FgColor & 0x0000FF) >> 4,  // R
            (FgColor & 0x00FF00) >> 8,  // G
            (FgColor & 0xFF0000) >> 16, // B
            (BgColor & 0x0000FF) >> 4,  // R
            (BgColor & 0x00FF00) >> 8,  // G
            (BgColor & 0xFF0000) >> 16  // B
        )
    );
    
    // Print the string
    gST->ConOut->OutputString(gST->ConOut, Buffer);
    
    // Reset text attributes
    gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK));
}
