#include <unoskrnl.h>

ACPI_HEADER *g_DsdtTable = NULL;
ACPI_SLEEP_INFO g_S5Info = {0, 0, FALSE};
ACPI_SLEEP_INFO g_S3Info = {0, 0, FALSE};
ACPI_POWER_REG g_AcpiPowerReg;

SINLINE VOID OUTW(IN USINT16 Port, IN USINT16 Value) {
    __asm__ __volatile__ (
        "outw %0, %1"
        :
        : "a"(Value), "Nd"(Port)
    );
}

BOOL AcpiValidateChecksum(IN VOID *Table, IN UINT Length) {
    USINT8 Sum = 0;
    USINT8 *bytes = (USINT8*)Table;

    for(UINT i = 0; i < Length; i++) {
        Sum += bytes[i];
    }

    return(Sum == 0);
}

INT AcpiParseFADT(IN ACPI_FADT *Fadt) {
    if (!Fadt) {
        serial_printf("FADT Pointer NULL\n");
        STATUS_INVALID;
    }

    if(!(Fadt->Header.Signature[0] == 'F' &&
         Fadt->Header.Signature[1] == 'A' &&
         Fadt->Header.Signature[2] == 'C' &&
         Fadt->Header.Signature[3] == 'P')) {
            serial_printf("[Error] Not a valid FACP Signature. Abort");
            return STATUS_INVALID;
        }
    g_AcpiPowerReg.PM1a_CNT_BLK = Fadt->Pm1aCntBlk;
    g_AcpiPowerReg.PM1b_CNT_BLK = Fadt->Pm1bCntBlk;

    // Debug info
    serial_printf("[ACPI] PM1a_CNT_BLK = %X\n", g_AcpiPowerReg.PM1a_CNT_BLK);
    serial_printf("[ACPI] PM1b_CNT_BLK = %X\n", g_AcpiPowerReg.PM1b_CNT_BLK);
    
    serial_printf("=== FADT Info ===\n");
    serial_printf("Revision: %d\n", Fadt->Header.Revision);
    serial_printf("Length: %d bytes\n", Fadt->Header.Length);
    serial_printf("OEM ID: %.6s\n", Fadt->Header.OemId);
    serial_printf("OEM Table ID: %.8s\n", Fadt->Header.OemTableId);
    
    serial_printf("FirmwareCtrl (FACS): 0x%08X\n", Fadt->FirmwareCtrl);
    serial_printf("DSDT Address:         0x%08X\n", Fadt->Dsdt);

    if(Fadt->Header.Revision >= 2) {
        if(Fadt->X_Firmware_Control != 0) {
            serial_printf("[Info] X_Firmware_ctrl(64 bit): %llu", Fadt->X_Firmware_Control);
        }
        if(Fadt->X_Dsdt != 0) {
            serial_printf("[Info] X_Dsdt (64 Bit): %llu", Fadt->X_Dsdt);
        }
    }

    if(!AcpiValidateChecksum(Fadt, Fadt->Header.Length)) {
        serial_printf("[Error] Fadt Checksum Invalid\n");
        return STATUS_INVALID;
    } else {
        serial_printf("[OK] Fadt checksum OK\n");
    }

    USINT64 dsdt_phys = (Fadt->Header.Revision >= 2 && Fadt->X_Dsdt != 0)
                        ? Fadt->X_Dsdt
                        : (USINT64)Fadt->Dsdt;

    g_DsdtTable = (ACPI_HEADER*)dsdt_phys;

    if (!(g_DsdtTable->Signature[0] == 'D' &&
          g_DsdtTable->Signature[1] == 'S' &&
          g_DsdtTable->Signature[2] == 'D' &&
          g_DsdtTable->Signature[3] == 'T')) {
        serial_printf("DSDT Signature Invalid: %s\n", g_DsdtTable->Signature);
        g_DsdtTable = NULL;
        return STATUS_INVALID;
    }

    if(!AcpiValidateChecksum(g_DsdtTable, g_DsdtTable->Length)) {
        serial_printf("[Error] DSDT Checksum Failed\n");
        g_DsdtTable = NULL;
        return STATUS_INVALID;
    } 

    if(g_DsdtTable) {
        AcpiScanForS5(g_DsdtTable);
    }

    serial_printf("[OK] DSDT Loaded at %x, length %u bytes\n", dsdt_phys, g_DsdtTable->Length);
    return STATUS_OK;

}

INT AcpiScanForS5(IN ACPI_HEADER *Dsdt) {
    if (!Dsdt) return STATUS_UNKNOWN;

    USINT8  *aml      = (USINT8*)Dsdt;
    USINT32  Len      = Dsdt->Length;

    for (USINT32 i = 0; i + 1 < Len; i++) {
        // 1) Deteksi NameOp + NameSeg "_S5_"
        if (aml[i]   == 0x08 &&
            aml[i+1] == '_' &&
            aml[i+2] == 'S' &&
            aml[i+3] == '5' &&
            aml[i+4] == '_') {
            serial_printf("[Info] _S5 found at offset 0x%X\n", i);

            // 2) Pastikan PackageOp ada di i+5
            if (i + 5 >= Len || aml[i+5] != 0x12) {
                serial_printf("[Warn] Found _S5 but no PackageOp\n");
                continue;
            }

            // 3) Scan byte by byte mulai dari i+6, cari dua 0x0A
            USINT8 seen = 0;
            for (USINT32 j = i + 6; j + 1 < Len && seen < 2; j++) {
                if (aml[j] == 0x0A) {
                    USINT8 val = aml[j+1] & 0x7F;
                    if (seen == 0) {
                        g_S5Info.SLP_TYPa = val;
                    } else {
                        g_S5Info.SLP_TYPb = val;
                    }
                    seen++;
                    j++; // lompat satu byte ke depan
                }
            }

            if (seen == 2) {
                g_S5Info.Found = TRUE;
                serial_printf("[Info] _S5 SLP_TYPa = 0x%X, SLP_TYPb = 0x%X\n",
                              g_S5Info.SLP_TYPa, g_S5Info.SLP_TYPb);
                return STATUS_OK;
            } else {
                serial_printf("[Warn] _S5 found but couldn't extract 2 bytes (got %u)\n", seen);
            }
        }
    }

    serial_printf("[Warning] _S5 method NOT FOUND\n");
    return STATUS_FAIL;
}



VOID AcpiShutdown() {
    if(!g_S5Info.Found) {
        serial_printf("[ACPI] No _S5 info, cannot shudown\n");
        return;
    }

    USINT16 slp_typa = (g_S5Info.SLP_TYPa << 10) & 0x1C00;
    USINT16 slp_typb = (g_S5Info.SLP_TYPb << 10) & 0x1C00;

    serial_printf("[ACPI] Shutdown initiated...\n");

    if(g_AcpiPowerReg.PM1a_CNT_BLK != 0) {
        OUTW(g_AcpiPowerReg.PM1a_CNT_BLK, slp_typb | SLP_EN);
    }
    if(g_AcpiPowerReg.PM1b_CNT_BLK != 0) {
        OUTW(g_AcpiPowerReg.PM1b_CNT_BLK, slp_typb | SLP_EN);
    }

    serial_printf("[ACPI] Shutdown failed? System still running...\n");

    // Optional infinite loop
    for (;;) __asm__ __volatile__("hlt");
}

VOID AcpiSleep() {
    if (!g_S3Info.Found) {
        serial_printf("[ACPI] No _S3 info, cannot sleep\n");
        return;
    }

    USINT16 slp_typa = (g_S3Info.SLP_TYPa << 10) & 0x1C00;
    USINT16 slp_typb = (g_S3Info.SLP_TYPb << 10) & 0x1C00;

    serial_printf("[ACPI] Entering SLEEP (S3)...\n");

    if (g_AcpiPowerReg.PM1a_CNT_BLK != 0)
        OUTW(g_AcpiPowerReg.PM1a_CNT_BLK, slp_typa | SLP_EN);
    if (g_AcpiPowerReg.PM1b_CNT_BLK != 0)
        OUTW(g_AcpiPowerReg.PM1b_CNT_BLK, slp_typb | SLP_EN);

    serial_printf("[ACPI] Sleep failed? Still running...\n");

    while (1) __asm__ __volatile__("hlt");
}


