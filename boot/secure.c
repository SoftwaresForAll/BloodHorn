#include <Uefi.h>
#include "compat.h"
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseCryptLib.h>
#include <Protocol/ImageAuthentication.h>
#include <Guid/ImageAuthentication.h>
#include <Guid/GlobalVariable.h>

EFI_STATUS EFIAPI VerifyImageSignature(
    IN CONST VOID    *ImageBuffer,
    IN UINTN         ImageSize,
    IN CONST UINT8   *PublicKey,
    IN UINTN         PublicKeySize
) {
    // Assume the signature is appended to the image: [image][signature]
    if (ImageSize < 256) return EFI_SECURITY_VIOLATION;
    UINTN DataSize = ImageSize - 256;
    CONST UINT8* Data = (CONST UINT8*)ImageBuffer;
    CONST UINT8* Signature = Data + DataSize;
    UINT8 Hash[32];
    if (!Sha256HashAll(Data, DataSize, Hash)) return EFI_SECURITY_VIOLATION;
    if (!RsaPkcs1Verify(PublicKey, PublicKeySize, Hash, sizeof(Hash), Signature, 256))
        return EFI_SECURITY_VIOLATION;
    return EFI_SUCCESS;
}

BOOLEAN EFIAPI IsSecureBootEnabled(VOID) {
    EFI_STATUS Status;
    UINT8 SecureBoot = 0;
    UINTN DataSize = sizeof(SecureBoot);
    Status = gRT->GetVariable(L"SecureBoot", &gEfiGlobalVariableGuid, NULL, &DataSize, &SecureBoot);
    return !EFI_ERROR(Status) && (SecureBoot == 1);
}

EFI_STATUS EFIAPI LoadAndVerifyKernel(
    IN  CHAR16  *FileName,
    OUT VOID    **ImageBuffer,
    OUT UINTN   *ImageSize
) {
    EFI_STATUS Status;
    VOID *Buffer = NULL;
    UINTN Size = 0;
    Status = ReadFile(FileName, &Buffer, &Size);
    if (EFI_ERROR(Status)) return Status;
    if (IsSecureBootEnabled()) {
        // Load public key from a secure variable
        UINT8 PublicKey[256];
        UINTN PublicKeySize = sizeof(PublicKey);
        Status = gRT->GetVariable(L"PK", &gEfiGlobalVariableGuid, NULL, &PublicKeySize, PublicKey);
        if (EFI_ERROR(Status)) {
            FreePool(Buffer);
            return Status;
        }
        Status = VerifyImageSignature(Buffer, Size, PublicKey, PublicKeySize);
        if (EFI_ERROR(Status)) {
            FreePool(Buffer);
            return Status;
        }
    }
    *ImageBuffer = Buffer;
    *ImageSize = Size;
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI ExecuteKernel(
    IN VOID     *ImageBuffer,
    IN UINTN    ImageSize,
    IN CHAR16   *CmdLine OPTIONAL
) {
    EFI_STATUS Status;
    EFI_HANDLE ImageHandle = NULL;
    Status = gBS->LoadImage(FALSE, gImageHandle, NULL, ImageBuffer, ImageSize, &ImageHandle);
    if (EFI_ERROR(Status)) return Status;
    if (CmdLine != NULL) {
        EFI_LOADED_IMAGE_PROTOCOL* LoadedImage = NULL;
        gBS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage);
        if (LoadedImage != NULL) {
            LoadedImage->LoadOptions = CmdLine;
            LoadedImage->LoadOptionsSize = (StrLen(CmdLine) + 1) * sizeof(CHAR16);
        }
    }
    Status = gBS->StartImage(ImageHandle, NULL, NULL);
    if (EFI_ERROR(Status)) gBS->UnloadImage(ImageHandle);
    return Status;
}