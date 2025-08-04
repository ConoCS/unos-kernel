#include <unoskrnl.h>

GLOBAL BOOT_INFO *bootInfo;

VOID init_acpi() {
    ACPI_MADT *Madt = (ACPI_MADT*)bootInfo->AcpiBootInform->ACPIMADT;
    if (Madt == NULL) {
        serial_print("ACPI MADT not found!\n");
        return;
    }

    serial_print("ACPI MADT found!\n");
    CHARA8 sig[5];
    memcpy(sig, &Madt->Header.Signature, 4);
    sig[4] = '\0';
    serial_printf("MADT Signature: %s\n", sig);
    serial_printf("MADT Length: %u\n", Madt->Header.Length);
    serial_printf("Local APIC Address: 0x%X\n", Madt->LocalApicAddress);
    serial_printf("MADT Flags: 0x%X\n", Madt->Flags);
    UEFIParseMADT(Madt);

    ACPI_FADT *Fadt = (ACPI_FADT*)bootInfo->AcpiBootInform->ACPIFADT;
    if (Fadt == NULL) {
        serial_printf("ACPI FADT not found!\n");
    } else {
        serial_print("ACPI FADT Found\n");

        CHARA8 Sig [5];
        memcpy(sig, &Fadt->Header.Signature, 4);
        sig[4] = '\0';
        serial_printf("FADT Signature: %s\n", sig);
        serial_printf("FADT Length: %u\n", Fadt->Header.Length);
        serial_printf("PM1a_CNT_BLK: 0x%X\n", Fadt->Pm1aCntBlk);

        AcpiParseFADT(Fadt);
    }
}

VOID apic_send_eoi() {
    volatile uint32_t* lapic_eoi = (uint32_t*)(0xFEE000B0);
    *lapic_eoi = 0;
}