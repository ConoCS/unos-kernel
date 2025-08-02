#include "madt.h"
#include "acpi.h"
#include "../Include/com.h"
#include "../boot/bootinfo.h"
#include <stdint.h>
#include "../Include/string.h"

extern BOOT_INFO *bootInfo;

void init_acpi() {
    ACPI_MADT *Madt = (ACPI_MADT*)bootInfo->AcpiBootInform->ACPIMADT;
    if (Madt == NULL) {
        serial_print("ACPI MADT not found!\n");
        return;
    }

    if (Madt->Header.Signature != ACPI_MADT_SIGNATURE) {
        serial_print("Invalid MADT Signature!\n");
    }
    serial_print("ACPI MADT found!\n");
    char sig[5];
    memcpy(sig, &Madt->Header.Signature, 4);
    sig[4] = '\0';
    serial_printf("MADT Signature: %s\n", sig);
    serial_printf("MADT Length: %u\n", Madt->Header.Length);
    serial_printf("Local APIC Address: 0x%X\n", Madt->LocalApicAddress);
    serial_printf("MADT Flags: 0x%X\n", Madt->Flags);
    UEFIParseMADT(Madt);
}

void apic_send_eoi() {
    volatile uint32_t* lapic_eoi = (uint32_t*)(0xFEE000B0);
    *lapic_eoi = 0;
}