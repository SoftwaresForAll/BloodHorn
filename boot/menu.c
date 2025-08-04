/*
 * BloodHorn Bootloader
 *
 * This file is part of BloodHorn and is licensed under the MIT License.
 * See the root of the repository for license details.
 */
#include <Uefi.h>
#include "compat.h"
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleTextInEx.h>
#include "../uefi/graphics.h"
#include "theme.h"
#include "localization.h"
#include "mouse.h"
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../config/config_ini.h"
#include "../config/config_json.h"
#include "../config/config_env.h"

// what the FUCK! NOT POSSIBLE BABY IM C GOD
// but you gotta admit one thing. THAT UR SMART !!!
// yes and that im the absoulute best at assembly
// ITS TRUE! i lost a brain cell while hearing this but i belive in u, lol
// btw do we keep the comments? just for the entertaiment of users? eh ppl don't want code they want effectivness so keep it
// say smth to the future people that are looking at this
// if you want to be good at your life. just be a dev and suffer
// did half life 3 release?  AND DID PORTAL 3 RELEASE??
// yea u reminded me
// yea that was it
// BTW people are thinking this is one person talking to himself
// a simple note. this is called vscode liveshare . AND IM NOT Autistic so please chill
// anyways bro quick commit and push cuz im gonan go afk for awhile
// SKADOOOOOOOOOOOSSH!

#define MAX_BOOT_ENTRIES 128
#define MAX_ENTRY_LENGTH 64
#define VISIBLE_MENU_ENTRIES 10

// Boot menu entry structure
typedef struct {
    CHAR16 Name[MAX_ENTRY_LENGTH];
    EFI_STATUS (*BootFunction)(VOID);
} BOOT_MENU_ENTRY;

// Global boot menu state
static BOOT_MENU_ENTRY BootEntries[MAX_BOOT_ENTRIES];
static UINTN BootEntryCount = 0;
static INTN SelectedEntry = 0;
static INTN MenuScrollOffset = 0;

// Hotkey mapping
static wchar_t BootEntryHotkeys[MAX_BOOT_ENTRIES];

// Helper: assign hotkeys (first unique letter)
static void AssignHotkeys() {
    bool used[256] = {0};
    for (UINTN i = 0; i < BootEntryCount; ++i) {
        const wchar_t* name = BootEntries[i].Name;
        for (int j = 0; name[j]; ++j) {
            wchar_t c = towlower(name[j]);
            if (c >= 32 && c < 128 && !used[c]) {
                BootEntryHotkeys[i] = c;
                used[c] = true;
                break;
            }
        }
        if (!BootEntryHotkeys[i]) BootEntryHotkeys[i] = 0;
    }
}

// Helper: load external localization file
static void LoadLocalizationFile(const char* lang_code) {
    char filename[64];
    snprintf(filename, sizeof(filename), "lang_%s.txt", lang_code);
    FILE* f = fopen(filename, "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char key[64]; wchar_t value[256];
        if (sscanf(line, "%63[^=]=%255ls", key, value) == 2) {
            AddLocalizedString(key, value);
        }
    }
    fclose(f);
}

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
    const struct BootMenuTheme* theme = GetBootMenuTheme();
    if (theme->background_image) {
        DrawImage(theme->background_image, 0, 0);
    } else {
        ClearScreen(theme->background_color);
    }
    INT32 ScreenWidth = GraphicsOutput->Mode->Info->HorizontalResolution;
    INT32 ScreenHeight = GraphicsOutput->Mode->Info->VerticalResolution;
    DrawRect(0, 0, ScreenWidth, 60, theme->header_color);
    INT32 MenuX = ScreenWidth / 4;
    INT32 MenuY = 100;
    INT32 MenuWidth = ScreenWidth / 2;
    INT32 MenuHeight = (VISIBLE_MENU_ENTRIES * 50) + 40;
    DrawRect(MenuX, MenuY, MenuWidth, MenuHeight, theme->header_color);
    INT32 TextX = MenuX + 20;
    INT32 TextY = MenuY + 20;
    const wchar_t* menu_title = GetLocalizedString("menu_title");
    PrintXY((ScreenWidth - StrLen(menu_title) * 10) / 2, 30, theme->selected_text_color, theme->header_color, L"%s", menu_title);
    // Draw visible entries
    for (UINTN i = MenuScrollOffset; i < BootEntryCount && i < MenuScrollOffset + VISIBLE_MENU_ENTRIES; i++) {
        if (i == (UINTN)SelectedEntry) {
            DrawRect(MenuX + 10, TextY - 5, MenuWidth - 20, 40, theme->highlight_color);
        }
        wchar_t entry_label[MAX_ENTRY_LENGTH+8];
        if (BootEntryHotkeys[i]) {
            swprintf(entry_label, sizeof(entry_label)/sizeof(wchar_t), L"(%c) %s", BootEntryHotkeys[i], BootEntries[i].Name);
        } else {
            swprintf(entry_label, sizeof(entry_label)/sizeof(wchar_t), L"%s", BootEntries[i].Name);
        }
        PrintXY(TextX, TextY, (i == (UINTN)SelectedEntry) ? theme->selected_text_color : theme->text_color, 0x00000000, L"%s", entry_label);
        TextY += 50;
    }
    // Up/down arrows if needed
    if (MenuScrollOffset > 0) {
        PrintXY(MenuX + MenuWidth - 40, MenuY + 10, theme->footer_color, 0x00000000, L"↑");
    }
    if (MenuScrollOffset + VISIBLE_MENU_ENTRIES < BootEntryCount) {
        PrintXY(MenuX + MenuWidth - 40, MenuY + MenuHeight - 30, theme->footer_color, 0x00000000, L"↓");
    }
    const wchar_t* instructions = GetLocalizedString("instructions");
    UINTN InstructionsWidth = StrLen(instructions) * 10;
    PrintXY((ScreenWidth - InstructionsWidth) / 2, MenuY + MenuHeight + 20, theme->footer_color, 0x00000000, L"%s", instructions);
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
    struct MouseState mouse = {0};
    // LoadThemeAndLanguageFromConfig(); // removed, now in main.c
    LoadLocalizationFile("en"); // TODO: use selected lang
    AssignHotkeys();
    Status = InitializeGraphics();
    if (EFI_ERROR(Status)) {
        gST->ConOut->ClearScreen(gST->ConOut);
    }
    if (BootEntryCount == 0) {
        AddBootEntry(L"Exit to UEFI Firmware", NULL);
    }
    const struct BootMenuTheme* theme = GetBootMenuTheme();
    InitMouse();
    while (TRUE) {
        if (!EFI_ERROR(Status)) {
            DrawBootMenu();
        } else {
            gST->ConOut->ClearScreen(gST->ConOut);
            Print(L"\r\n  BloodHorn Boot Menu\r\n\r\n");
            for (UINTN i = 0; i < BootEntryCount; i++) {
                Print(L"  %c %s\r\n", (i == (UINTN)SelectedEntry) ? '>' : ' ', BootEntries[i].Name);
            }
            Print(L"\r\n  Use arrow keys to select, Enter to boot, ESC to exit");
        }
        // Mouse support
        GetMouseState(&mouse);
        INT32 MenuX = GraphicsOutput->Mode->Info->HorizontalResolution / 4;
        INT32 MenuY = 100;
        INT32 MenuWidth = GraphicsOutput->Mode->Info->HorizontalResolution / 2;
        INT32 TextX = MenuX + 20;
        INT32 TextY = MenuY + 20;
        for (UINTN i = MenuScrollOffset; i < BootEntryCount && i < MenuScrollOffset + VISIBLE_MENU_ENTRIES; i++) {
            INT32 entryY = TextY + (i - MenuScrollOffset) * 50;
            if (mouse.x >= TextX && mouse.x < TextX + MenuWidth - 40 && mouse.y >= entryY && mouse.y < entryY + 40) {
                SelectedEntry = i;
                if (mouse.left_button) {
                    if (BootEntries[SelectedEntry].BootFunction != NULL) {
                        return BootEntries[SelectedEntry].BootFunction();
                    }
                    return EFI_SUCCESS;
                }
            }
        }
        // Wait for key or mouse event
        WaitList[0] = gST->ConIn->WaitForKey;
        Status = gBS->WaitForEvent(1, WaitList, &WaitIndex);
        if (EFI_ERROR(Status)) {
            continue;
        }
        Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
        if (EFI_ERROR(Status)) {
            continue;
        }
        // Hotkey support
        if (Key.UnicodeChar) {
            wchar_t c = towlower(Key.UnicodeChar);
            for (UINTN i = 0; i < BootEntryCount; ++i) {
                if (BootEntryHotkeys[i] == c) {
                    SelectedEntry = i;
                    break;
                }
            }
        }
        switch (Key.UnicodeChar) {
            case CHAR_CARRIAGE_RETURN:
                if (BootEntries[SelectedEntry].BootFunction != NULL) {
                    return BootEntries[SelectedEntry].BootFunction();
                }
                return EFI_SUCCESS;
            case 0:
                switch (Key.ScanCode) {
                    case SCAN_UP:
                        if (SelectedEntry > 0) {
                            SelectedEntry--;
                            if (SelectedEntry < MenuScrollOffset) MenuScrollOffset--;
                        } else {
                            SelectedEntry = BootEntryCount - 1;
                            MenuScrollOffset = (BootEntryCount > VISIBLE_MENU_ENTRIES) ? BootEntryCount - VISIBLE_MENU_ENTRIES : 0;
                        }
                        break;
                    case SCAN_DOWN:
                        if ((UINTN)SelectedEntry < BootEntryCount - 1) {
                            SelectedEntry++;
                            if (SelectedEntry >= MenuScrollOffset + VISIBLE_MENU_ENTRIES) MenuScrollOffset++;
                        } else {
                            SelectedEntry = 0;
                            MenuScrollOffset = 0;
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
    
    // Format the string
    VA_START(Args, Format);
    UnicodeVSPrint(Buffer, sizeof(Buffer), Format, Args);
    VA_END(Args);
    
    // Set cursor position
    gST->ConOut->SetCursorPosition(gST->ConOut, (UINTN)X, (UINTN)Y);
    
    // Map 0xRRGGBB to EFI colors (basic 16 colors, best effort)
    // Using only intensity and basic colors (not true RGB)
    UINT8 fg = EFI_LIGHTGRAY;
    UINT8 bg = EFI_BLACK;
    // You can improve color mapping here if needed
    
    gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(fg, bg));
    
    // Print the string
    gST->ConOut->OutputString(gST->ConOut, Buffer);
    
    // Reset to default colors (white on black)
    gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK));
}
