/** @file
  GP54 speaker test utility for IT8786 SIO.

  Usage:
    SPKGP54 -1  Use GPIO polarity toggling, matching the MemoryInit SPKOUT path.
    SPKGP54 -2  Use Simple I/O GPO output toggling.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#define SIO_CONFIG_INDEX             0x2E
#define SIO_CONFIG_DATA              0x2F

#define SIO_LDN_SEL_REGISTER         0x07
#define SIO_GPIO_LDN                 0x07
#define SIO_ACTIVATE_REGISTER        0x30

#define SIO_NORMAL_BASE_MSB_REGISTER 0x60
#define SIO_NORMAL_BASE_LSB_REGISTER 0x61
#define SIO_SIMPLE_BASE_MSB_REGISTER 0x62
#define SIO_SIMPLE_BASE_LSB_REGISTER 0x63

#define SIO_GP5X_SELECT_REGISTER     0x29
#define SIO_GP5X_POLARITY_REGISTER   0xB4
#define SIO_GP5X_SIMPLE_IO_REGISTER  0xC4
#define SIO_GP5X_OUTPUT_REGISTER     0xCC

#define SIO_NORMAL_BASE              0x0A10
#define SIO_SIMPLE_BASE              0x0A00
#define SIO_GP5X_DATA_PORT           (SIO_SIMPLE_BASE + 0x04)
#define GP54_BIT                     BIT4

#define SPEAKER_SETTLE_US            10000
#define PWM_HALF_PERIOD_US           500
#define PWM_CYCLES                   100

STATIC
VOID
SioEnterConfig (
  VOID
  )
{
  IoWrite8 (SIO_CONFIG_INDEX, 0x87);
  IoWrite8 (SIO_CONFIG_INDEX, 0x01);
  IoWrite8 (SIO_CONFIG_INDEX, 0x55);

#if (SIO_CONFIG_INDEX == 0x2E)
  IoWrite8 (SIO_CONFIG_INDEX, 0x55);
#else
  IoWrite8 (SIO_CONFIG_INDEX, 0xAA);
#endif
}

STATIC
VOID
SioExitConfig (
  VOID
  )
{
  IoWrite8 (SIO_CONFIG_INDEX, 0x02);
  IoWrite8 (SIO_CONFIG_DATA, 0x02);
}

STATIC
VOID
SioSelectGpioLdn (
  VOID
  )
{
  IoWrite8 (SIO_CONFIG_INDEX, SIO_LDN_SEL_REGISTER);
  IoWrite8 (SIO_CONFIG_DATA, SIO_GPIO_LDN);
}

STATIC
UINT8
SioReadRegister (
  IN UINT8  Register
  )
{
  IoWrite8 (SIO_CONFIG_INDEX, Register);
  return IoRead8 (SIO_CONFIG_DATA);
}

STATIC
VOID
SioWriteRegister (
  IN UINT8  Register,
  IN UINT8  Value
  )
{
  IoWrite8 (SIO_CONFIG_INDEX, Register);
  IoWrite8 (SIO_CONFIG_DATA, Value);
}

STATIC
VOID
SioSetRegisterBits (
  IN UINT8  Register,
  IN UINT8  SetBits
  )
{
  SioWriteRegister (Register, (UINT8)(SioReadRegister (Register) | SetBits));
}

STATIC
VOID
SioClearRegisterBits (
  IN UINT8  Register,
  IN UINT8  ClearBits
  )
{
  SioWriteRegister (Register, (UINT8)(SioReadRegister (Register) & (UINT8)~ClearBits));
}

STATIC
VOID
DelayUs (
  IN UINTN  Microseconds
  )
{
  gBS->Stall (Microseconds);
}

STATIC
VOID
Gp54WriteOutput (
  IN BOOLEAN  High
  )
{
  UINT8  Value;

  Value = IoRead8 (SIO_GP5X_DATA_PORT);
  if (High) {
    Value = (UINT8)(Value | GP54_BIT);
  } else {
    Value = (UINT8)(Value & (UINT8)~GP54_BIT);
  }

  IoWrite8 (SIO_GP5X_DATA_PORT, Value);
}

STATIC
VOID
Gp54SetPolarity (
  IN BOOLEAN  Invert
  )
{
  if (Invert) {
    SioSetRegisterBits (SIO_GP5X_POLARITY_REGISTER, GP54_BIT);
  } else {
    SioClearRegisterBits (SIO_GP5X_POLARITY_REGISTER, GP54_BIT);
  }
}

STATIC
VOID
Gp54Initialize (
  VOID
  )
{
  SioSelectGpioLdn ();

  //
  // Keep the pin inactive before enabling the GPIO logical device.
  //
  Gp54SetPolarity (FALSE);

  //
  // Normal Run base is kept separate from Simple I/O base. Mode -2 uses the
  // Simple I/O window, so CR62/CR63 must point to SIO_SIMPLE_BASE.
  //
  SioWriteRegister (SIO_NORMAL_BASE_MSB_REGISTER, (UINT8)(SIO_NORMAL_BASE >> 8));
  SioWriteRegister (SIO_NORMAL_BASE_LSB_REGISTER, (UINT8)(SIO_NORMAL_BASE & 0xFF));
  SioWriteRegister (SIO_SIMPLE_BASE_MSB_REGISTER, (UINT8)(SIO_SIMPLE_BASE >> 8));
  SioWriteRegister (SIO_SIMPLE_BASE_LSB_REGISTER, (UINT8)(SIO_SIMPLE_BASE & 0xFF));

  SioSetRegisterBits (SIO_ACTIVATE_REGISTER, BIT0);

  //
  // GP54 belongs to GPIO group 5 bit 4.
  //
  SioSetRegisterBits (SIO_GP5X_SELECT_REGISTER, GP54_BIT);
  SioSetRegisterBits (SIO_GP5X_SIMPLE_IO_REGISTER, GP54_BIT);
  Gp54WriteOutput (FALSE);
  SioSetRegisterBits (SIO_GP5X_OUTPUT_REGISTER, GP54_BIT);
  Gp54WriteOutput (FALSE);
}

STATIC
VOID
Gp54SpeakerOff (
  VOID
  )
{
  Gp54SetPolarity (FALSE);
  Gp54WriteOutput (FALSE);
}

STATIC
VOID
Gp54BeepByPolarity (
  VOID
  )
{
  UINTN  Index;

  Gp54SpeakerOff ();
  DelayUs (SPEAKER_SETTLE_US);

  for (Index = 0; Index < PWM_CYCLES; Index++) {
    Gp54SetPolarity (TRUE);
    DelayUs (PWM_HALF_PERIOD_US);
    Gp54SetPolarity (FALSE);
    DelayUs (PWM_HALF_PERIOD_US);
  }

  Gp54SpeakerOff ();
}

STATIC
VOID
Gp54BeepByGpo (
  VOID
  )
{
  UINTN  Index;

  Gp54WriteOutput (FALSE);
  DelayUs (SPEAKER_SETTLE_US);

  for (Index = 0; Index < PWM_CYCLES; Index++) {
    Gp54WriteOutput (TRUE);
    DelayUs (PWM_HALF_PERIOD_US);
    Gp54WriteOutput (FALSE);
    DelayUs (PWM_HALF_PERIOD_US);
  }

  Gp54WriteOutput (FALSE);
}

STATIC
VOID
PrintUsage (
  VOID
  )
{
  Print (L"Usage:\r\n");
  Print (L"  SPKGP54 -1    GPIO polarity toggling test\r\n");
  Print (L"  SPKGP54 -2    GPO output toggling test\r\n");
}

INTN
EFIAPI
ShellAppMain (
  IN UINTN   Argc,
  IN CHAR16  **Argv
  )
{
  if (Argc != 2) {
    PrintUsage ();
    return EFI_INVALID_PARAMETER;
  }

  if (StrCmp (Argv[1], L"-1") == 0) {
    Print (L"SPKGP54 -1: GPIO polarity toggling.\r\n");
    SioEnterConfig ();
    Gp54Initialize ();
    Gp54BeepByPolarity ();
    Gp54SpeakerOff ();
    SioExitConfig ();
  } else if (StrCmp (Argv[1], L"-2") == 0) {
    Print (L"SPKGP54 -2: GPO output toggling.\r\n");
    SioEnterConfig ();
    Gp54Initialize ();
    Gp54SpeakerOff ();
    SioExitConfig ();
    Gp54BeepByGpo ();
    Gp54WriteOutput (FALSE);
  } else {
    PrintUsage ();
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}
