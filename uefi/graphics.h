#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include <Uefi.h>

// Function to initialize graphics
EFI_STATUS
InitializeGraphics(VOID);

// Function to draw a rectangle
EFI_STATUS
DrawRect(
    IN UINT32 X,
    IN UINT32 Y,
    IN UINT32 Width,
    IN UINT32 Height,
    IN UINT32 Color
);

// Function to clear the screen with a color
EFI_STATUS
ClearScreen(
    IN UINT32 Color
);

// Function to get the screen dimensions
VOID
GetScreenDimensions(
    OUT UINT32 *Width,
    OUT UINT32 *Height
);

#endif // _GRAPHICS_H_
