#include <unoskrnl.h>

IOAPIC_INFO g_ioapic;
ISO_INFO g_iso[MAX_ISO_ENTRIES];
INT g_iso_count = 0;
USINT32 IoAPICAddress = 0;

VOID UEFIParseMADT(ACPI_MADT *Madt) {
    USINT32 lapic_addr = Madt->LocalApicAddress;
    serial_printf("Local APIC Address: 0x%X\n", lapic_addr);

    USINT8 *entry = Madt->Entries;
    USINT8 *end = ((USINT8*)Madt) + Madt->Header.Length;
    
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
                g_ioapic.ioapic_id = ioapic->IoApicId;
                g_ioapic.ioapic_addr = ioapic->IoApicAddress;
                g_ioapic.gsi_base = ioapic->GlobalSystemInterruptBase;
                IoAPICAddress = g_ioapic.ioapic_addr;   
                serial_printf("RAW IOAPIC ENTRY: ");
                for (int i = 0; i < ioapic->Header.Length; i++) {
                    serial_printf("%X ", *((USINT8*)ioapic + i));
                }
                serial_printf("\n");
                if (g_ioapic.ioapic_addr == 0xFEC00000 && g_ioapic.gsi_base != 0) {
                serial_printf("IOAPIC at 0xFEC00000 but GSI base is %u? Forcing to 0\n", g_ioapic.gsi_base);
                g_ioapic.gsi_base = 0;
            }
                break;
            }

            case 2: {
                MADT_ISO *iso = (MADT_ISO*)entry;
                serial_printf("ISO: BusSource=%d, IrqSource=%d, GlobalSystemInterrupt=%d, Flags=0x%X\n",
                              iso->BusSource, iso->IrqSource, iso->GlobalSystemInterrupt, iso->Flags);
                if(g_iso_count < MAX_ISO_ENTRIES) {
                    g_iso[g_iso_count].irq_source = iso->IrqSource;
                    g_iso[g_iso_count].gsi = iso->GlobalSystemInterrupt;
                    g_iso[g_iso_count].flags = iso->Flags;
                    g_iso_count++;
                    serial_printf("ISO Info[%d]: IrqSource=%d, GSI=%d, Flags=0x%X\n",
                                  g_iso_count - 1, iso->IrqSource, iso->GlobalSystemInterrupt, iso->Flags); 

                }
                break;
            }
            default:
                serial_printf("Unknown MADT entry type: %d\n", hdr->Type);
                break;
        }
        entry += hdr->Length;
    }
}

USINT32 UEFIFindGSIForIRQ(USINT8 irq) {
    for(int i = 0; i < g_iso_count; i++) {
        if(g_iso[i].irq_source == irq) {
            return g_iso[i].gsi;
        }
    }
    return irq;
}

USINT32 UEFIIOAPICRead(USINT32 reg) {
    IOAPIC_REGSEL(g_ioapic.ioapic_addr) = reg;
    return IOAPIC_IOWIN(g_ioapic.ioapic_addr);
}

VOID IOAPICWrite(USINT32 reg, USINT32 value) {
    IOAPIC_REGSEL(g_ioapic.ioapic_addr) = reg;
    IOAPIC_IOWIN(g_ioapic.ioapic_addr) = value;
}

VOID IOAPICRedirectIRQ(IN USINT8 irq, IN USINT8 vector, IN USINT8 lapic_id) {
    USINT32 gsi = UEFIFindGSIForIRQ(irq);
    USINT32 index = gsi * 2;

    IOAPICWrite(0x10 + index + 1, lapic_id << 24);

    USINT32 low = 0;
    low |= vector;
    low &= ~(1 << 16);
    low |= (0 << 11);
    low |= (0 << 13);
    low |= (0 << 15);

    IOAPICWrite(0x10 + index, low);
}

VOID InitIOAPIC(){
    serial_printf("Initializing IOAPIC...\n");
    IOAPICRedirectIRQ(1, 0x21, 0);
    IOAPICRedirectIRQ(12, 0x2C, 0);
    serial_printf("IOAPIC initialized\n");
}

VOID LAPICEnable() {
    LAPIC_REG(LAPIC_SVR) = 0xFF | (1 << 8);
}

VOID Wait10ms() {
    // Masked one-shot
    LAPIC_REG(LAPIC_LVT_TIMER) = LAPIC_WAIT_VECTOR | (1 << 16);
    LAPIC_REG(LAPIC_TIMER_DIVIDE) = 0b1010; // divide by 128
    LAPIC_REG(LAPIC_INITIAL_COUNT) = 120000; // coba turun dikit

    // Tunggu hingga habis
    while (LAPIC_REG(LAPIC_CURRENT_COUNT) != 0) {
        __asm__ volatile ("pause");
    }
}
USINT32 CalibrateLAPICBusHz() {
    // Set LAPIC timer ke one-shot dan maximal count
    LAPIC_REG(LAPIC_TIMER_DIVIDE) = 0xB; // divide-by-1
    LAPIC_REG(LAPIC_LVT_TIMER) = (1 << 16); // Masked
    LAPIC_REG(LAPIC_INITIAL_COUNT) = 0xFFFFFFFF;

    // Delay selama 10ms, misalnya pakai I/O wait loop
    Wait10ms(); // Buat delay ini sesingkat mungkin dan stabil

    USINT32 remaining = LAPIC_REG(LAPIC_CURRENT_COUNT);
    return (0xFFFFFFFF - remaining) * 100; // hitung Hz
}

VOID SetupLapicTimer(USINT32 bus_hz) {
    USINT32 count = bus_hz / 100; // 100 tick per detik

    // 1) Divide-by-1 (0b1011)
    LAPIC_REG(LAPIC_TIMER_DIVIDE) = 0b1011;

    // 2) Set LVT_TIMER: vector 0x20, periodic mode (bit 17 = 1), unmask (bit 16 = 0)
    LAPIC_REG(LAPIC_LVT_TIMER) = (TIMER_VECTOR /*0x20*/)
                              | (1 << 17)       /* periodic */
                              /*| (0 << 16)*/; /* unmask */

    // 3) Set initial count
    LAPIC_REG(LAPIC_INITIAL_COUNT) = count;
}

