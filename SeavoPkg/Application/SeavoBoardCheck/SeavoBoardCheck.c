/** @file
  SEAVO board detection UEFI application.

  Reads CMOS bytes 0x7B..0x7F and checks whether they match
  "SEAVO" XOR 's', following the SEAVO soft specification check.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/IoLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>

#define CMOS_INDEX_PORT       0x70
#define CMOS_DATA_PORT        0x71
#define CMOS_NMI_DISABLE_BIT  0x80
#define SEAVO_CMOS_BASE       0x7B
#define SEAVO_SIGNATURE_SIZE  5
#define SEAVO_XOR_KEY         's'
#define UPDATE_SCRIPT_NAME    L"up.nsh"

STATIC
UINT8
ReadCmos8 (
  IN UINT8  Address
  )
{
  UINT8  IndexPortValue;

  IndexPortValue = IoRead8 (CMOS_INDEX_PORT);
  IoWrite8 (
    CMOS_INDEX_PORT,
    (UINT8)((IndexPortValue & CMOS_NMI_DISABLE_BIT) | Address)
    );

  return IoRead8 (CMOS_DATA_PORT);
}

STATIC
BOOLEAN
IsSeavoBoard (
  VOID
  )
{
  STATIC CONST UINT8  Signature[SEAVO_SIGNATURE_SIZE] = { 'S', 'E', 'A', 'V', 'O' };
  UINT8              Index;
  UINT8              Value;
  UINT8              Expected;

  for (Index = 0; Index < SEAVO_SIGNATURE_SIZE; Index++) {
    Value    = ReadCmos8 ((UINT8)(SEAVO_CMOS_BASE + Index));
    Expected = (UINT8)(Signature[Index] ^ SEAVO_XOR_KEY);
    if (Value != Expected) {
      Print (
        L"Mismatch at CMOS 0x%02x: read 0x%02x, expected 0x%02x\r\n",
        (UINT8)(SEAVO_CMOS_BASE + Index),
        Value,
        Expected
        );
      return FALSE;
    }
  }

  return TRUE;
}

/**
  Application entry point.

  @retval EFI_SUCCESS             SEAVO board signature was detected.
  @retval EFI_SECURITY_VIOLATION  SEAVO board signature was not detected.
**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_STATUS  ScriptStatus;
  CHAR16      CommandLine[] = UPDATE_SCRIPT_NAME;

  if (IsSeavoBoard ()) {
    Print (L"SEAVO board detected. Launching " UPDATE_SCRIPT_NAME L".\r\n");
    Status = ShellExecute (&ImageHandle, CommandLine, TRUE, NULL, &ScriptStatus);
    if (EFI_ERROR (Status)) {
      Print (L"ERROR: Could not execute " UPDATE_SCRIPT_NAME L": %r\r\n", Status);
      return Status;
    }

    if (EFI_ERROR (ScriptStatus)) {
      Print (L"ERROR: " UPDATE_SCRIPT_NAME L" returned %r\r\n", ScriptStatus);
    }

    return ScriptStatus;
  }

  Print (L"SEAVO board not detected. BIOS update blocked.\r\n");
  return EFI_SECURITY_VIOLATION;
}
