#include <Uefi.h>
#include "compat.h"
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DevicePath.h>

/**
  Locates the root directory of the boot device.
  
  @param[out] RootFs    Pointer to the root file system interface.
  
  @retval EFI_SUCCESS   The root directory was found successfully.
  @retval Other         An error occurred.
**/
EFI_STATUS
GetRootFileSystem(
    OUT EFI_FILE_PROTOCOL **RootFs
) {
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
    EFI_DEVICE_PATH *DevicePath = NULL;
    EFI_DEVICE_PATH *FileDevicePath = NULL;
    EFI_HANDLE DeviceHandle = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;

    // Get the loaded image protocol
    Status = gBS->HandleProtocol(
        gImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (VOID **)&LoadedImage
    );
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Get the device path protocol
    Status = gBS->HandleProtocol(
        LoadedImage->DeviceHandle,
        &gEfiDevicePathProtocolGuid,
        (VOID **)&DevicePath
    );
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Get the file system protocol
    Status = gBS->HandleProtocol(
        LoadedImage->DeviceHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID **)&FileSystem
    );
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Open the root directory
    return FileSystem->OpenVolume(FileSystem, RootFs);
}

/**
  Reads a file from the boot device.
  
  @param[in]  FileName    The name of the file to read.
  @param[out] Buffer      Pointer to store the allocated buffer containing file contents.
  @param[out] FileSize    Pointer to store the size of the file.
  
  @retval EFI_SUCCESS     The file was read successfully.
  @retval Other           An error occurred.
**/
EFI_STATUS
ReadFile(
    IN  CHAR16            *FileName,
    OUT VOID              **Buffer,
    OUT UINTN             *FileSize
) {
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL *RootFs = NULL;
    EFI_FILE_PROTOCOL *FileHandle = NULL;
    EFI_FILE_INFO *FileInfo = NULL;
    UINTN InfoSize;
    UINTN ReadSize;
    VOID *FileBuffer = NULL;

    // Get the root file system
    Status = GetRootFileSystem(&RootFs);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Open the file
    Status = RootFs->Open(
        RootFs,
        &FileHandle,
        FileName,
        EFI_FILE_MODE_READ,
        0
    );
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Get file info size
    Status = FileHandle->GetInfo(
        FileHandle,
        &gEfiFileInfoGuid,
        &InfoSize,
        NULL
    );
    if (Status != EFI_BUFFER_TOO_SMALL) {
        FileHandle->Close(FileHandle);
        return Status;
    }

    // Allocate buffer for file info
    FileInfo = AllocatePool(InfoSize);
    if (FileInfo == NULL) {
        FileHandle->Close(FileHandle);
        return EFI_OUT_OF_RESOURCES;
    }

    // Get file info
    Status = FileHandle->GetInfo(
        FileHandle,
        &gEfiFileInfoGuid,
        &InfoSize,
        FileInfo
    );
    if (EFI_ERROR(Status)) {
        FreePool(FileInfo);
        FileHandle->Close(FileHandle);
        return Status;
    }

    // Allocate buffer for file contents
    FileBuffer = AllocatePool((UINTN)FileInfo->FileSize);
    if (FileBuffer == NULL) {
        FreePool(FileInfo);
        FileHandle->Close(FileHandle);
        return EFI_OUT_OF_RESOURCES;
    }

    // Read the file
    ReadSize = (UINTN)FileInfo->FileSize;
    Status = FileHandle->Read(
        FileHandle,
        &ReadSize,
        FileBuffer
    );
    
    // Clean up
    FreePool(FileInfo);
    FileHandle->Close(FileHandle);
    
    if (EFI_ERROR(Status)) {
        FreePool(FileBuffer);
        return Status;
    }
    
    // Return the buffer and size
    *Buffer = FileBuffer;
    *FileSize = ReadSize;
    
    return EFI_SUCCESS;
}
