#ifndef UNOS_ACPI_FADT_H
#define UNOS_ACPI_FADT_H

#include <unostype.h>
#include "acpi.h"
#include "madt.h"
#define SLP_EN (1 << 13)

typedef struct {
        USINT8 AddressSpace;
        USINT8 RegisterBitWidth;
        USINT8 RegisterBitOffset;
        USINT8 AccessSize;
        USINT64 Address;
} __attribute__((packed)) GENERIC_ADDRESS_STRUCTURE;

typedef struct ACPI_FADT {
    ACPI_HEADER Header;

    USINT32 FirmwareCtrl;          // Alamat kontrol firmware
    USINT32 Dsdt;                 // Alamat DSDT

    USINT8  Reserved;            // Reserved, harus 0

    USINT8 PreferredPMProfile;
    USINT16 SciInt;
    USINT32 SmiCMD;
    USINT8 AcpiEnable;           // ACPI Enable Register
    USINT8 AcpiDisable;          // ACPI Disable Register
    USINT8 S4BiosReq;            // S4BIOS Request Register
    USINT8 PStateCnt;            // P-States Control Register

    USINT32 PM1aEvtBlk;      // Alamat PM1a Event Block
    USINT32 PM1bEvtBlk;
    USINT32 Pm1aCntBlk;
    USINT32 Pm1bCntBlk;
    USINT32 Pm2CntBlk;
    USINT32 PmTmrBlk;
    USINT32 Gpe0Blk;
    USINT32 Gpe1Blk;

    USINT8 Pm1EvtLen;
    USINT8 Pm1CntLen;
    USINT8 Pm2CntLen;
    USINT8 PmTmrLen;
    USINT8 Gpe0BlkLen;
    USINT8 Gpe1BlkLen;
    USINT8 Gpe1Base;
    USINT8 CstCnt;

    USINT16 PLvl2Lat;
    USINT16 PLvl3Lat;
    USINT16 FlushSize;
    USINT16 FlushStride;
    USINT8 DutyOffset;
    USINT8 DutyWidth;
    USINT8 DayAlrm; 
    USINT8 MonAlrm;
    USINT8 Century;
    USINT16 IapcBootArch;
    USINT8 Reserved2;

    USINT32 Flags;

    GENERIC_ADDRESS_STRUCTURE ResetReg;

    USINT8 ResetValue;
    USINT8 Reserved3[3];

    USINT64 X_Firmware_Control;
    USINT64 X_Dsdt;
} __attribute__((packed)) ACPI_FADT;

typedef struct {
    USINT8 SLP_TYPa;
    USINT8 SLP_TYPb;
    BOOL Found;
} ACPI_SLEEP_INFO;

typedef struct {
    USINT16 PM1a_CNT_BLK;
    USINT16 PM1b_CNT_BLK;
} ACPI_POWER_REG;

GLOBAL ACPI_POWER_REG g_AcpiPowerReg;
GLOBAL ACPI_SLEEP_INFO g_S5Info;
GLOBAL ACPI_HEADER *g_DsdtTable;

BOOL AcpiValidateChecksum(IN VOID *Table, IN UINT Length);
INT AcpiParseFADT(IN ACPI_FADT *Fadt);
INT AcpiScanForS5(IN ACPI_HEADER *Dsdt);
VOID AcpiShutdown();
VOID AcpiSleep();

#endif