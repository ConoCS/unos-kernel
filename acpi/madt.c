#include "madt.h"
#include "acpi.h"
#include "../Include/com.h"
#include <stdint.h>
#include <stddef.h>

void UEFIParseMADT(ACPI_MADT *Madt) {
    uint32_t lapic_addr = Madt->LocalApicAddress;
    serial_printf("Local APIC Address: 0x%X\n", lapic_addr);

    uint8_t *entry = Madt->Entries;
    uint8_t *end = ((uint8_t*)Madt) + Madt->Header.Length;
    
    while(entry < end) {
        MADT_ENTRY_HEADER *hdr = (MADT_ENTRY_HEADER*)entry;

        if (hdr->Length < sizeof(MADT_ENTRY_HEADER)) {
            serial_printf("ERROR: Invalid MADT entry: Length=%u (too small)\n", hdr->Length);
            break; // Hindari infinite loop
        }

        // Cegah overrun
        if (entry + hdr->Length > end) {
            serial_printf("ERROR: MADT entry goes beyond MADT size\n");
            break;
        }

        switch(hdr->Type) {
            case 0: {
                MADT_LAPIC *lapic = (MADT_LAPIC*)entry;
                serial_printf("LAPIC: ProcessorId=%d, ApicId=%d, Flags=0x%X\n",
                              lapic->ProcessorId, lapic->ApicId, lapic->Flags);
                serial_printf("Type=%u, Length=%u\n", hdr->Type, hdr->Length);
                break;
            }
            case 1: {
                MADT_IOAPIC *ioapic = (MADT_IOAPIC*)entry;
                serial_printf("IOAPIC: IoApicId=%d, IoApicAddress=0x%X, GlobalSystemInterruptBase=%d\n",
                              ioapic->IoApicId, ioapic->IoApicAddress, ioapic->GlobalSystemInterruptBase);
                serial_printf("Type=%u, Length=%u\n", hdr->Type, hdr->Length);
                break;
            }

            case 2: {
                MADT_ISO *iso = (MADT_ISO*)entry;
                serial_printf("ISO: BusSource=%d, IrqSource=%d, GlobalSystemInterrupt=%d, Flags=0x%X\n",
                              iso->BusSource, iso->IrqSource, iso->GlobalSystemInterrupt, iso->Flags);
                break;
            }
            default:
                serial_printf("Unknown MADT entry type: %d\n", hdr->Type);
                break;
        }
        entry += hdr->Length;
    }
}
