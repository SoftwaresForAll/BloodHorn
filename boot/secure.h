#ifndef BLOODHORN_SECURE_BOOT_H
#define BLOODHORN_SECURE_BOOT_H

#include <Base.h>
#include <Uefi.h>
#include "compat.h"

// Function to verify an image signature
EFI_STATUS EFIAPI
VerifyImageSignature(
    IN CONST VOID    *ImageBuffer,
    IN UINTN         ImageSize,
    IN CONST UINT8   *PublicKey,
    IN UINTN         PublicKeySize
);
// Function to check if Secure Boot is enabled
BOOLEAN EFIAPI
IsSecureBootEnabled(VOID);

// Function to load and verify a kernel image
EFI_STATUS EFIAPI
LoadAndVerifyKernel(
    IN  CHAR16  *FileName,
    OUT VOID    **ImageBuffer,
    OUT UINTN   *ImageSize
);

// Function to execute a loaded kernel
EFI_STATUS EFIAPI
ExecuteKernel(
    IN VOID     *ImageBuffer,
    IN UINTN    ImageSize,
    IN CHAR16   *CmdLine OPTIONAL
); // 
#endif // _SECURE_BOOT_H_
