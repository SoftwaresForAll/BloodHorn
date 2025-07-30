[Defines]
  PLATFORM_NAME           = BloodHorn
  PLATFORM_GUID           = 12345678-1234-1234-1234-1234567890AB
  PLATFORM_VERSION        = 0.1
  DSC_SPECIFICATION       = 0x00010005
  OUTPUT_DIRECTORY        = Build/BloodHorn
  SUPPORTED_ARCHITECTURES = X64
  BUILD_TARGETS           = DEBUG|RELEASE
  SKUID_IDENTIFIER        = DEFAULT

[LibraryClasses]
  UefiLib                 = MdePkg/Library/UefiLib/UefiLib.inf
  BaseLib                 = MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib           = MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  MemoryAllocationLib     = MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DebugLib                = MdePkg/Library/DebugLib/DebugLib.inf
  PrintLib                = MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PcdLib                  = MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

[Packages]
  MdePkg/MdePkg.dec

[Components]
  BloodHorn/BloodHorn.inf
