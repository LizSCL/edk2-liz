/** @file
  This sample application bases on HelloWorld PCD setting
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2006 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/SmbusLib.h>
#include <Library/PciLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/ShellCEntryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ShellLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/SimpleFileSystem.h>
#include <string.h>
#include <stdio.h>

#define Ver_Main 02
#define Ver_Sub1 06
#define Ver_Sub2 01

#define BLOCK_WR 0x8
#define BLOCK_DAT_LEGNTH 0x0D
#define Max_Length_Bin 0x2000

BOOLEAN DEBUG_MODE = FALSE;
UINT8 SMB_FUN;
BOOLEAN SMB_IO_ACCESS = TRUE;

// extern EFI_RUNTIME_SERVICES  *gRT;
// extern EFI_SYSTEM_TABLE   *gST;
extern EFI_BOOT_SERVICES *gBS;

#define OEM_PCI_ADDRESS(Bus, Device, Function, Register) \
	((1 << 31) | (((Bus) & 0xFF) << 16) | (((Device) & 0x1F) << 11) | (((Function) & 0x07) << 8) | ((Register) & 0xFF))

#define TOLOWER(x) ((x) | 0x20)
#define isxdigit(c) (('0' <= (c) && (c) <= '9') || ('a' <= (c) && (c) <= 'f') || ('A' <= (c) && (c) <= 'F'))
#define isdigit(c) ('0' <= (c) && (c) <= '9')

typedef enum
{
	PLATFORM_UNKNOWN = 0,
	PLATFORM_INTEL,
	PLATFORM_AMD
} PLATFORM_TYPE;

PLATFORM_TYPE
DetectPlatform(
	VOID)
{
	UINT32 Eax, Ebx, Ecx, Edx;

	AsmCpuid(0, &Eax, &Ebx, &Ecx, &Edx);

	// "GenuineIntel"
	if (Ebx == SIGNATURE_32('G', 'e', 'n', 'u') &&
		Edx == SIGNATURE_32('i', 'n', 'e', 'I') &&
		Ecx == SIGNATURE_32('n', 't', 'e', 'l'))
	{
		return PLATFORM_INTEL;
	}

	// "AuthenticAMD"
	if (Ebx == SIGNATURE_32('A', 'u', 't', 'h') &&
		Edx == SIGNATURE_32('e', 'n', 't', 'i') &&
		Ecx == SIGNATURE_32('c', 'A', 'M', 'D'))
	{
		return PLATFORM_AMD;
	}

	return PLATFORM_UNKNOWN;
}

unsigned long strtoul(const CHAR16 *str)
{
	unsigned long result = 0, value;
	int i = 0;
	unsigned int base = 10;
	char *cp = (char *)AllocatePool(20 * sizeof(char)); // allocate 20 bytes
	for (i = 0; str[i] != '\0'; i++)
	{
		cp[i] = (char)str[i];
	}
	cp[i] = '\0';
	if (*cp == '0')
	{
		base = 8;
		cp++;
		if ((TOLOWER(*cp) == 'x') && isxdigit(cp[1]))
		{
			cp++;
			base = 16;
		}
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp - '0' : TOLOWER(*cp) - 'a' + 10) < base)
	{
		result = result * base + value;
		cp++;
	}
	return result;
}

void display_buffer_hex(UINT8 *buffer, UINT32 size)
{
	UINT32 i, j, k;

	for (i = 0; i < size; i += 16)
	{
		Print(L"\n  %08x  ", i);
		for (j = 0, k = 0; k < 16; j++, k++)
		{
			if (i + j < size)
			{
				Print(L"%02x", buffer[i + j]);
			}
			else
			{
				Print(L"  ");
			}
			Print(L" ");
		}
		Print(L" ");
		for (j = 0, k = 0; k < 16; j++, k++)
		{
			if (i + j < size)
			{
				if ((buffer[i + j] < 32) || (buffer[i + j] > 126))
				{
					Print(L".");
				}
				else
				{
					Print(L"%c", buffer[i + j]);
				}
			}
		}
	}
	Print(L"\n");
}

UINTN Strlen(CHAR16 *string)
{
	UINTN length = 0;
	while (*string++)
		length++;
	return length;
}

CHAR16 *Strcat(CHAR16 *string1, CHAR16 *string2)
{
	CHAR16 *dest = string1;

	string1 += Strlen(string1);

	while (*string2)
	{
		*string1++ = *string2++;
	}
	*string1 = 0;
	return dest;
}

CHAR16 *Strcut(CHAR16 *str, CHAR16 c)
{
	while (*str != c)
	{
		str++;
	}
	return str;
}
BOOLEAN StrcmpNoCase(CHAR16 *Str1, CHAR16 *Str2)
{
	CHAR16 Char1;
	CHAR16 Char2;
	while (*Str1)
	{
		if (*Str1 != *Str2)
		{
			Char1 = (*Str1 >= 0x41 && *Str1 <= 0x5a) ? *Str1 : (*Str1 >= 0x61 && *Str1 <= 0x7a) ? *Str1 - 0x20
																								: 0xff;
			Char2 = (*Str2 >= 0x41 && *Str2 <= 0x5a) ? *Str2 : (*Str2 >= 0x61 && *Str2 <= 0x7a) ? *Str2 - 0x20
																								: 0;
			if (Char1 != Char2)
				return FALSE;
		}
		Str1++;
		Str2++;
	}
	return (*Str1 == *Str2);
}
/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/

typedef UINT16 WORD;
typedef UINT32 DWORD;

static WORD SMBus_Port;
static WORD SMB_HST_STS;
static WORD SMB_HST_CNT;
static WORD SMB_HST_CMD;
static WORD SMB_HST_TSA;
static WORD SMB_HST_D0;
static WORD SMB_HST_D1;
static WORD SMB_HST_BLOCK_DB;
static WORD SMB_AUX_CTL;

static UINT8 SMB_BYTE_OPCODE = 0x48;
static UINT8 SMB_BLOCK_OPCODE = 0x54;

WORD GetSMBusBaseAddr()
{

	int i;
	unsigned int value;
	UINT8 fun[3] = {1, 3, 4};

	if (DEBUG_MODE)
	{
		Print(L"******************************\nStart GetSMBusBaseAddr()\n");
	}
	for (i = 0; i < 3; i++)
	{
		IoWrite32(0xCF8, (DWORD)OEM_PCI_ADDRESS(0, 31, fun[i], 0x08));
		value = IoRead32(0xCFC);
		if (DEBUG_MODE)
		{
			Print(L"BUS:0x00/DEV:0x1F/FUN:0x%x\noffset:0x08/value:0x%x\n", fun[i], value);
		}
		if ((value != 0xffffffff) && ((value >> 16) == (0x0c05))) // value high 8 bits is 0x0C05 or not
		{
			SMB_FUN = fun[i];
			// Judge SMB IO Access Register
			IoWrite32(0xCF8, (DWORD)OEM_PCI_ADDRESS(0, 31, fun[i], 0x04));
			value = IoRead32(0xCFC);
			SMB_IO_ACCESS = value & 0x01;
			if (!SMB_IO_ACCESS)
			{
				if (DEBUG_MODE)
					Print(L"Warning!!The Register IOSE(I/O Space Enable) for SMBUS Controller is zero by default.\n");
				IoWrite32(0xCFC, value | 0x01);
			}
			IoWrite32(0xCF8, (DWORD)OEM_PCI_ADDRESS(0, 31, fun[i], 0x20));
			value = IoRead32(0xCFC);
			if (DEBUG_MODE)
			{
				Print(L"BUS:0x00/DEV:0x1F/FUN:0x%x\noffset:0x20/value:0x%x\n", fun[i], value);
				Print(L"END GetSMBusBaseAddr()\n##############################\n");
			}
			return (WORD)(value & 0x0000FFE0);
		}
	}
	if (DEBUG_MODE)
	{
		Print(L"END GetSMBusBaseAddr()\n##############################\n");
	}
	return 0;
}
BOOLEAN InitSmbusBar()
{
	if (DEBUG_MODE)
	{
		Print(L"******************************\nStart InitSmbusBar()\n");
	}
	SMBus_Port = GetSMBusBaseAddr(); // ApolloLake
	if (SMBus_Port == 0)
	{
		Print(L"Cannot get SMBus Base Address\n");
		if (DEBUG_MODE)
		{
			Print(L"END InitSmbusBar()\n##############################\n");
		}
		return FALSE;
	}
	Print(L"SMBus Base Address:  0x%x\n", SMBus_Port);

	SMB_HST_STS = SMBus_Port + 0;
	SMB_HST_CNT = SMBus_Port + 2;
	SMB_HST_CMD = SMBus_Port + 3;
	SMB_HST_TSA = SMBus_Port + 4;
	SMB_HST_D0 = SMBus_Port + 5;
	SMB_HST_D1 = SMBus_Port + 6;
	SMB_HST_BLOCK_DB = SMBus_Port + 7;
	SMB_AUX_CTL = SMBus_Port + 0x0D;
	if (DEBUG_MODE)
	{
		Print(L"END InitSmbusBar()\n##############################\n");
	}
	return TRUE;
}

VOID WaitSMBReady(VOID)
{

	while ((IoRead8(SMB_HST_STS)) & 0x01)
	{
		gBS->Stall(10);
	}
}
UINT8 SvSmbReadByte(UINT8 DEVADD, UINT8 index)
{
	UINT8 ida = 0;
	UINT8 DID = DEVADD; // Device Address

	IoWrite8(SMB_HST_D0, 0x00);
	IoWrite8(SMB_AUX_CTL, 0x00);

	IoWrite8(SMB_HST_STS, 0x1E);

	IoWrite8(SMB_HST_TSA, DID | 0x01);

	IoWrite8(SMB_HST_CMD, index);

	IoWrite8(SMB_HST_CNT, SMB_BYTE_OPCODE);

	WaitSMBReady();

	ida = IoRead8(SMB_HST_D0);

	return ida;
}
BOOLEAN SvSmbWriteByte(UINT8 DEVADD, UINT8 index, UINT8 data)
{
	UINT8 DID = DEVADD; // Device Address

	IoWrite8(SMB_HST_D0, 0x00);
	IoWrite8(SMB_AUX_CTL, 0x00);

	IoWrite8(SMB_HST_STS, 0x1E);

	IoWrite8(SMB_HST_TSA, DID & 0xFE);

	IoWrite8(SMB_HST_CMD, index);

	IoWrite8(SMB_HST_D0, data);

	IoWrite8(SMB_HST_CNT, SMB_BYTE_OPCODE);

	WaitSMBReady();
	return TRUE;
}
BOOLEAN SvSmbWriteBlock(UINT8 DEVADD, UINT8 index, UINT8 *DATA, UINT8 iCount)
{
	UINT8 i = 0;
	UINT8 DID = DEVADD;

	IoWrite8(SMB_HST_STS, 0x1E);

	IoWrite8(SMB_HST_TSA, DID & 0xFE);

	IoWrite8(SMB_HST_CMD, index);

	IoWrite8(SMB_HST_D0, iCount);

	IoWrite8(SMB_AUX_CTL, 0x02);

	for (i = 0; i < iCount; i++)
	{
		IoWrite8(SMB_HST_BLOCK_DB, DATA[i]);
	}

	IoWrite8(SMB_HST_CNT, SMB_BLOCK_OPCODE);
	WaitSMBReady();
	return TRUE;
}

UINT8 E2PROM_Read_Byte_SV(unsigned int eeprom_addr)
{
	static unsigned int high_addr = 0xFF;
	if (high_addr != (eeprom_addr >> 8))
	{
		high_addr = eeprom_addr >> 8;
		SvSmbWriteByte(0x42, 0xb2, (UINT8)(eeprom_addr >> 8)); // High address bit4-0
	}
	SvSmbWriteByte(0x42, 0xb3, (UINT8)(eeprom_addr & 0xff)); // Low 8 bit address
	SvSmbWriteByte(0x42, 0xb5, 0x02);						 // read command

	return SvSmbReadByte(0x42, 0xb6);
}

int HavePara(UINTN argc, CHAR16 **argv, CHAR16 *str)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		if (StrcmpNoCase(str, argv[i]))
		{
			return i;
		}
	}
	return -1;
}

BOOLEAN IsNotNum(CHAR16 *str)
{
	BOOLEAN sixteenFlag = FALSE;
	int i = 0;
	char *cp = (char *)AllocatePool(20 * sizeof(char)); // allocate 20 bytes
	for (i = 0; str[i] != '\0'; i++)
	{
		cp[i] = (char)str[i];
	}
	cp[i] = '\0';
	i = 0;
	if (*(cp + 1) == 'x' || *(cp + 1) == 'X')
	{
		if (*cp != '0')
		{
			return TRUE;
		}
		i = 2;
		sixteenFlag = TRUE;
	}
	for (; *(cp + i) != '\0'; i++)
	{
		if (sixteenFlag)
		{
			if (isxdigit(*(cp + i)))
			{
				continue;
			}
			else
			{
				return TRUE;
			}
		}
		else
		{
			if (isdigit(*(cp + i)))
			{
				continue;
			}
			else
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
void printfPass(void)
{
	int i;
	CHAR16 *pass[] = {L"`%@@@@@@@@&$!'                                                          `",
					  L"'&##############!                                                       `",
					  L"'&###&:  `;&#####%.                                                     `",
					  L"'&###&'     :####@:       `':'.             .'::`           `':'`       `",
					  L"'&###&'     '&###@:  '&##########%.     '&#########;    ;#########&'    `",
					  L"'&###&'    .%####%.  '&#%:. .!@###&'   ;####&:. `!$;   %####%` .'|%'    `",
					  L"'&##############%%.            '@###!   |####|         `$###@:           `",
					  L"'&###########@!       .!@##########!   '&#######%`     ;#######@!.      `",
					  L"'&###&'             `&####&!:`;@###!     `%########|     '&#######@:    `",
					  L"'&###&'             |###$.    :@###!          '&####;         !####&`   `",
					  L"'&###&'             %####!  .|#####!   !%`    .%###@: `%!.    '&###$.   `",
					  L"'&###&'             `&#######@$@###!   |##########&'  `$##########%.    `",
					  L" `'''`                 :%$%;.  ''''.    `;|$$$|;.       '!%$$%|'        `"};
	for (i = 0; i < 13; i++)
	{
		Print(L"%s\n", pass[i]);
	}
}
void printfFail(void)
{
	int i;
	CHAR16 *fail[] = {L"'!!!!!!!!!!!!;.                    `$###&'   !####;        .",
					  L"|############&'                    :@####;   !####;        .",
					  L"|####&%%%%%%%!.                      `:'     !####;        .",
					  L"|####;                                       !####;        .",
					  L"|####;            !##########%.    `$###&'   !####;        .",
					  L"|####!           .%##&|;!$####@:   `$###&'   !####;        .",
					  L"|############;            .$###%.  `$###&'   !####;        .",
					  L"|############;     .;$@########$`  `$###&'   !####;        .",
					  L"|####;           !#####@$%|&###$`  `$###&'   !####;        .",
					  L"|####;          ;####:    .%###$`  `$###&'   !####;        .",
					  L"|####;          ;####!   `$####$`  `$###&'   !####;        .",
					  L"|####;          .%########@@###$`  `$###&'   !####;        .",
					  L":%%%%'            `|###@!. ;%%%;.  .;%%%!.   :%%%%'        ."};
	for (i = 0; i < 13; i++)
	{
		Print(L"%s\n", fail[i]);
	}
}

INTN
	EFIAPI
	ShellAppMain(
		IN UINTN Argc,
		IN CHAR16 **Argv)
{
	PLATFORM_TYPE Platform;

	Platform = DetectPlatform();

	switch (Platform)
	{
	case PLATFORM_INTEL:
		Print(L"Platform detected: Intel\n");
		break;

	case PLATFORM_AMD:
		Print(L"Platform detected: AMD\n");
		break;

	default:
		Print(L"Platform detected: Unknown CPU vendor\n");
		break;
	}

	UINT8 BLOCK_DAT[BLOCK_DAT_LEGNTH];
	unsigned int E2P_Address;
	unsigned int BurnLength;
	unsigned char Ver_Cur, Ver_Cur_E2PROM, Ver_New;
	unsigned long int i;
	unsigned long int j;

	EFI_STATUS Status;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
	EFI_FILE_PROTOCOL *Root;
	EFI_FILE_PROTOCOL *FileHandle = 0;
	UINTN FileBufSize;
	UINT8 *FileBuffer;

	Print(L"This tool is used for updating TR5511 image\n");
	Print(L"Software version: %d.%d.%d\n", Ver_Main, Ver_Sub1, Ver_Sub2);

	if (Argc < 3)
	{
		Print(L"Error: Parameter Error, Program exit\n");
		return EFI_INVALID_PARAMETER;
	}

	if (HavePara(Argc, Argv, L"/debug") >= 0)
	{
		Print(L"Debug Mode: ON\n");
		DEBUG_MODE = TRUE;
	}

	if (!InitSmbusBar())
	{
		Print(L"Init SmbusBar Error, Program exit\n");
		if (!SMB_IO_ACCESS)
		{
			IoWrite32(0xCF8, (DWORD)OEM_PCI_ADDRESS(0, 31, SMB_FUN, 0x04));
			IoWrite32(0xCFC, IoRead32(0xCFC) & 0xFFFFFFFE);
		}
		return EFI_INVALID_PARAMETER;
	}

	int fIndex = HavePara(Argc, Argv, L"/f");			  // Write the whole eeprom according to the bin file
	int rIndex = HavePara(Argc, Argv, L"/r");			  // Read and print the eeprom data according to the address
	int mcuVerIndex = HavePara(Argc, Argv, L"/mcucheck"); // check mcu version
	if (fIndex >= 0)
	{
		CHAR16 *fileName = Argv[fIndex + 1];
		CHAR16 *path = (CHAR16 *)ShellGetCurrentDir(NULL);
		if (path[Strlen(path) - 1] != L'\\')
		{
			path = Strcat(path, L"\\");
		}
		fileName = Strcat(path, fileName);
		Print(L"Image name is: %s\n", fileName);

		Status = gBS->LocateProtocol(&gEfiSimpleFileSystemProtocolGuid, NULL, (VOID **)&SimpleFileSystem);
		if (EFI_ERROR(Status))
		{
			Print(L"Cannot find EFI_SIMPLE_FILE_SYSTEM_PROTOCOL \r\n");
			if (!SMB_IO_ACCESS)
			{
				IoWrite32(0xCF8, (DWORD)OEM_PCI_ADDRESS(0, 31, SMB_FUN, 0x04));
				IoWrite32(0xCFC, IoRead32(0xCFC) & 0xFFFFFFFE);
			}
			return Status;
		}
		Status = SimpleFileSystem->OpenVolume(SimpleFileSystem, &Root);
		if (EFI_ERROR(Status))
		{
			Print(L"OpenVolume error \r\n");
			if (!SMB_IO_ACCESS)
			{
				IoWrite32(0xCF8, (DWORD)OEM_PCI_ADDRESS(0, 31, SMB_FUN, 0x04));
				IoWrite32(0xCFC, IoRead32(0xCFC) & 0xFFFFFFFE);
			}
			return Status;
		}
		Status = Root->Open(Root,
							&FileHandle,
							Strcut(fileName, L'\\'),
							EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
							0);
		if (EFI_ERROR(Status) || (FileHandle == 0))
		{
			Print(L"Open error \r\n");
			if (!SMB_IO_ACCESS)
			{
				IoWrite32(0xCF8, (DWORD)OEM_PCI_ADDRESS(0, 31, SMB_FUN, 0x04));
				IoWrite32(0xCFC, IoRead32(0xCFC) & 0xFFFFFFFE);
			}
			return Status;
		}

		Status = FileHandle->SetPosition(FileHandle, 0);
		FileBufSize = Max_Length_Bin;
		FileBuffer = AllocatePool(Max_Length_Bin);
		Status = FileHandle->Read(FileHandle, &FileBufSize, FileBuffer);
		if (EFI_ERROR(Status))
		{
			Print(L"Read error \r\n");
			if (!SMB_IO_ACCESS)
			{
				IoWrite32(0xCF8, (DWORD)OEM_PCI_ADDRESS(0, 31, SMB_FUN, 0x04));
				IoWrite32(0xCFC, IoRead32(0xCFC) & 0xFFFFFFFE);
			}
			return Status;
		}
		Print(L"\nReading %s file... \r\n", fileName);
		// display_buffer_hex(FileBuffer, Max_Length_Bin);

		// CLOSE MCU
		SvSmbWriteByte(0x42, 0xB1, 0x08);
		SvSmbWriteByte(0x42, 0xB1, 0x08);
		SvSmbWriteByte(0x42, 0xB1, 0x08);
		SvSmbWriteByte(0x42, 0xB1, 0x08);
		SvSmbWriteByte(0x42, 0xB1, 0x08);

		Ver_New = FileBuffer[0x112F];
		Ver_Cur = SvSmbReadByte(0x46, 0xC0);
		Ver_Cur_E2PROM = E2PROM_Read_Byte_SV(0x112F);

		Print(L"Current E2PROM MCU version: V0x%x\n", Ver_Cur);
		Print(L"Current E2PROM MCU version: V0x%x\n", Ver_Cur_E2PROM);
		Print(L"The New E2PROM MCU version: V0x%x\n", Ver_New);

		BurnLength = Max_Length_Bin / BLOCK_WR;

		Print(L"Start update TR5511 image:          ");
		for (i = 0x0; i < BurnLength; i++)
		{
			E2P_Address = i * BLOCK_WR;

			BLOCK_DAT[0] = 0x0A;						// 冗余数据 固定 0x0A
			BLOCK_DAT[1] = (E2P_Address & 0xFF00) >> 8; // eeprom 地址高位
			BLOCK_DAT[2] = (E2P_Address & 0x00FF) >> 0; // eeprom 地址低位

			for (j = 0; j < BLOCK_WR; j++)
			{
				BLOCK_DAT[3 + j] = FileBuffer[i * BLOCK_WR + j]; // eeprom 8 bytes 数据
			}

			SvSmbWriteBlock(0x42, 0x00, BLOCK_DAT, 0x0B);

			Print(L"\b\b\b\b\b\b\b\b\b%4d/%4d", E2P_Address, Max_Length_Bin);

			gBS->Stall(4000);
		}
		Print(L"\b\b\b\b\b\b\b\b\b%4d/%4d\n", Max_Length_Bin, Max_Length_Bin);
		// Check All
		if (HavePara(Argc, Argv, L"/check") >= 0)
		{
			Print(L"Start to read eeprom:          ");
			UINT8 *checkBuff = AllocatePool(Max_Length_Bin);
			for (i = 0; i < Max_Length_Bin; i++)
			{
				checkBuff[i] = E2PROM_Read_Byte_SV(i);
				Print(L"\b\b\b\b\b\b\b\b\b%4d/%4d", i, Max_Length_Bin);
			}
			Print(L"\b\b\b\b\b\b\b\b\b%4d/%4d\n", Max_Length_Bin, Max_Length_Bin);

			Print(L"Start to check eeprom:          ");
			for (i = 0; i < Max_Length_Bin; i++)
			{
				if (FileBuffer[i] != checkBuff[i])
				{
					Print(L"\nCheck ERROR!\neeprom_addr:0x%x\nThe data of file:0x%x\nThe data of rom:0x%x\nProgram exit\n", i, FileBuffer[i], checkBuff[i]);
					if (!SMB_IO_ACCESS)
					{
						IoWrite32(0xCF8, (DWORD)OEM_PCI_ADDRESS(0, 31, SMB_FUN, 0x04));
						IoWrite32(0xCFC, IoRead32(0xCFC) & 0xFFFFFFFE);
					}
					return EFI_INVALID_PARAMETER;
				}
				else
				{
					Print(L"\b\b\b\b\b\b\b\b\b%4d/%4d", i, Max_Length_Bin);
				}
			}
			Print(L"\b\b\b\b\b\b\b\b\b%4d/%4d\n", Max_Length_Bin, Max_Length_Bin);
			Print(L"Check eeprom successfully\n");
		}
		gBS->FreePool(FileBuffer);
		Print(L"E2PROM update finished\n");
	}
	else if (rIndex >= 0)
	{
		if (Argc < rIndex + 3 || IsNotNum(Argv[rIndex + 1]) || IsNotNum(Argv[rIndex + 2]))
		{
			Print(L"Error: Parameter Error, Program exit\n");
			if (!SMB_IO_ACCESS)
			{
				IoWrite32(0xCF8, (DWORD)OEM_PCI_ADDRESS(0, 31, SMB_FUN, 0x04));
				IoWrite32(0xCFC, IoRead32(0xCFC) & 0xFFFFFFFE);
			}
			return EFI_INVALID_PARAMETER;
		}
		UINT16 index = (UINT16)strtoul(Argv[rIndex + 1]);
		unsigned long length = (UINT16)strtoul(Argv[rIndex + 2]);
		Print(L"DevID:0x42\nIndex:0x%x\nLength:0x%x\n", index, length);
		FileBuffer = AllocatePool(Max_Length_Bin);
		Print(L"Start Read TR5511 image:          ");
		for (i = 0; i < length; i++)
		{
			FileBuffer[i] = E2PROM_Read_Byte_SV(i + index);
			Print(L"\b\b\b\b\b\b\b\b\b%4d/%4d", i, length);
		}
		Print(L"\b\b\b\b\b\b\b\b\b%4d/%4d\n", length, length);
		if (FileBuffer != NULL)
		{
			if (length <= Max_Length_Bin)
			{
				display_buffer_hex(FileBuffer, length);
			}
			else
			{
				Print(L"Error: Parameter Error, Program exit\n");
				if (!SMB_IO_ACCESS)
				{
					IoWrite32(0xCF8, (DWORD)OEM_PCI_ADDRESS(0, 31, SMB_FUN, 0x04));
					IoWrite32(0xCFC, IoRead32(0xCFC) & 0xFFFFFFFE);
				}
				return EFI_INVALID_PARAMETER;
			}
		}
	}
	else if (mcuVerIndex >= 0)
	{
		if (IsNotNum(Argv[mcuVerIndex + 1]))
		{
			Print(L"Error: Parameter Error, Program exit\n");
			if (!SMB_IO_ACCESS)
			{
				IoWrite32(0xCF8, (DWORD)OEM_PCI_ADDRESS(0, 31, SMB_FUN, 0x04));
				IoWrite32(0xCFC, IoRead32(0xCFC) & 0xFFFFFFFE);
			}
			return EFI_INVALID_PARAMETER;
		}
		UINT8 version = E2PROM_Read_Byte_SV(0x112F);
		UINT16 checkversion = (UINT16)strtoul(Argv[mcuVerIndex + 1]);
		Print(L"MCU Version Address: 0x112F\nMCU Check Version: 0x%x\n", version);
		if (checkversion == version)
		{
			printfPass();
		}
		else
		{
			printfFail();
		}
	}
	if (!SMB_IO_ACCESS)
	{
		IoWrite32(0xCF8, (DWORD)OEM_PCI_ADDRESS(0, 31, SMB_FUN, 0x04));
		IoWrite32(0xCFC, IoRead32(0xCFC) & 0xFFFFFFFE);
	}
	return EFI_SUCCESS;
}
