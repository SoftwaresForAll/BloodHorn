#ifndef _SECURE_BOOT_H_
#define _SECURE_BOOT_H_

#include <Uefi.h>

// Function to verify an image signature
EFI_STATUS
VerifyImageSignature(
    IN CONST VOID    *ImageBuffer,
    IN UINTN         ImageSize,
    IN CONST UINT8   *PublicKey,
    IN UINTN         PublicKeySize
);

// Function to check if Secure Boot is enabled
BOOLEAN
IsSecureBootEnabled(VOID);

// Function to load and verify a kernel image
EFI_STATUS
LoadAndVerifyKernel(
    IN  CHAR16  *FileName,
    OUT VOID    **ImageBuffer,
    OUT UINTN   *ImageSize
);

// Function to execute a loaded kernel
EFI_STATUS
ExecuteKernel(
    IN VOID     *ImageBuffer,
    IN UINTN    ImageSize,
    IN CHAR16   *CmdLine OPTIONAL
);

#endif // _SECURE_BOOT_H_
