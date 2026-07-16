## @file
#  SEAVO board utility build description.
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
##

[Defines]
  PLATFORM_NAME                  = SeavoPkg
  PLATFORM_GUID                  = 1EB4E6B8-5D79-4D75-982E-CF3E9A0C740B
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/SeavoPkg
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

!include MdePkg/MdeLibs.dsc.inc

[LibraryClasses]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|8000
  gEfiMdePkgTokenSpaceGuid.PcdUefiFileHandleLibPrintBufferSize|8000
  gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|TRUE
  gEfiShellPkgTokenSpaceGuid.PcdShellPrintBufferSize|8000

[Components]
  SeavoPkg/Application/SeavoBoardCheck/SeavoBoardCheck.inf
  SeavoPkg/Application/SPKGP54/SPKGP54.inf
