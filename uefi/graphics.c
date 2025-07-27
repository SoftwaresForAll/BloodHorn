#include <Uefi.h>
#include "compat.h"
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>

// Global graphics context
EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput = NULL;

/**
  Initializes the graphics output.
  
  @retval EFI_SUCCESS   The graphics output was initialized successfully.
  @retval Other         An error occurred.
**/
EFI_STATUS
InitializeGraphics() {
    EFI_STATUS Status;
    
    // Locate the graphics output protocol
    Status = gBS->LocateProtocol(
        &gEfiGraphicsOutputProtocolGuid,
        NULL,
        (VOID **)&GraphicsOutput
    );
    
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    // Query current video mode
    UINT32 CurrentMode = GraphicsOutput->Mode->Mode;
    UINTN SizeOfInfo = 0;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info = NULL;
    
    // Get current mode information
    Status = GraphicsOutput->QueryMode(
        GraphicsOutput,
        CurrentMode,
        &SizeOfInfo,
        &Info
    );
    
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    // Set a suitable video mode (preferably 1024x768 if available)
    UINT32 BestMode = 0;
    UINT32 BestWidth = 0;
    UINT32 BestHeight = 0;
    
    // Find the best available mode
    for (UINT32 i = 0; i < GraphicsOutput->Mode->MaxMode; i++) {
        Status = GraphicsOutput->QueryMode(
            GraphicsOutput,
            i,
            &SizeOfInfo,
            &Info
        );
        
        if (EFI_ERROR(Status)) {
            continue;
        }
        
        // Prefer 1024x768, but take the highest resolution available
        if (Info->HorizontalResolution * Info->VerticalResolution > BestWidth * BestHeight) {
            BestWidth = Info->HorizontalResolution;
            BestHeight = Info->VerticalResolution;
            BestMode = i;
        }
    }
    
    // Set the best mode
    if (BestMode != CurrentMode) {
        Status = GraphicsOutput->SetMode(GraphicsOutput, BestMode);
        if (EFI_ERROR(Status)) {
            return Status;
        }
    }
    
    return EFI_SUCCESS;
}

/**
  Draws a rectangle on the screen.
  
  @param[in] X          The X coordinate of the top-left corner.
  @param[in] Y          The Y coordinate of the top-left corner.
  @param[in] Width      The width of the rectangle.
  @param[in] Height     The height of the rectangle.
  @param[in] Color      The color of the rectangle in 0xRRGGBB format.
  
  @retval EFI_SUCCESS   The rectangle was drawn successfully.
  @retval Other         An error occurred.
**/
EFI_STATUS
DrawRect(
    IN UINT32 X,
    IN UINT32 Y,
    IN UINT32 Width,
    IN UINT32 Height,
    IN UINT32 Color
) {
    if (GraphicsOutput == NULL) {
        return EFI_NOT_READY;
    }
    
    // Get the framebuffer information
    UINT32 *Framebuffer = (UINT32 *)GraphicsOutput->Mode->FrameBufferBase;
    UINT32 PixelsPerScanline = GraphicsOutput->Mode->Info->PixelsPerScanLine;
    
    // Ensure the rectangle is within bounds
    if (X >= GraphicsOutput->Mode->Info->HorizontalResolution ||
        Y >= GraphicsOutput->Mode->Info->VerticalResolution) {
        return EFI_INVALID_PARAMETER;
    }
    
    // Clamp the width and height to the screen bounds
    if (X + Width > GraphicsOutput->Mode->Info->HorizontalResolution) {
        Width = GraphicsOutput->Mode->Info->HorizontalResolution - X;
    }
    if (Y + Height > GraphicsOutput->Mode->Info->VerticalResolution) {
        Height = GraphicsOutput->Mode->Info->VerticalResolution - Y;
    }
    
    // Draw the rectangle
    for (UINT32 y = Y; y < Y + Height; y++) {
        for (UINT32 x = X; x < X + Width; x++) {
            Framebuffer[y * PixelsPerScanline + x] = Color;
        }
    }
    
    return EFI_SUCCESS;
}

/**
  Clears the screen with the specified color.
  
  @param[in] Color    The color to clear the screen with in 0xRRGGBB format.
  
  @retval EFI_SUCCESS   The screen was cleared successfully.
  @retval Other         An error occurred.
**/
EFI_STATUS
ClearScreen(
    IN UINT32 Color
) {
    if (GraphicsOutput == NULL) {
        return EFI_NOT_READY;
    }
    
    return DrawRect(
        0,
        0,
        GraphicsOutput->Mode->Info->HorizontalResolution,
        GraphicsOutput->Mode->Info->VerticalResolution,
        Color
    );
}
