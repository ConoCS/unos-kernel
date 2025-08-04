#ifndef ACPI_MADT_H
#define ACPI_MADT_H

#include <unostype.h>
#include "acpi.h"

#define MAX_ISO_ENTRIES 24
#define IOAPIC_REGSEL(addr) (*(volatile USINT32*)(addr))
#define IOAPIC_IOWIN(addr)  (*(volatile USINT32*)((addr) + 0x10))
#define LAPIC_BASE 0xFEE00000
#define LAPIC_REG(offset) (*(volatile USINT32 *)(LAPIC_BASE + (offset)))
#define LAPIC_TIMER_DIVIDE 0x3E0
#define LAPIC_LVT_TIMER 0x320
#define LAPIC_INITIAL_COUNT 0x380
#define LAPIC_CURRENT_COUNT 0x390 
#define LAPIC_WAIT_VECTOR 0x31 // vector yang tidak dipakai
#define TIMER_VECTOR 0x20
#define SCALE_FACTOR 2  // misal interrupt 50×/detik → ≈100× skala

#define LAPIC_SVR   0xF0
#define LAPIC_EOI   0xB0

typedef struct {
    ACPI_HEADER Header;
    uint32_t LocalApicAddress;
    uint32_t Flags;
    uint8_t Entries[];
} __attribute__((packed)) ACPI_MADT;

typedef struct {
    uint8_t Type;
    uint8_t Length;
} __attribute__((packed)) MADT_ENTRY_HEADER;

typedef struct {
    MADT_ENTRY_HEADER Header;
    uint8_t  ProcessorId;
    uint8_t  ApicId;
    uint32_t Flags;
} __attribute__((packed)) MADT_LAPIC;

typedef struct {
    MADT_ENTRY_HEADER Header;
    uint8_t  IoApicId;
    uint8_t  Reserved;
    uint32_t IoApicAddress;
    uint32_t GlobalSystemInterruptBase;
} __attribute__((packed)) MADT_IOAPIC;

typedef struct {
    MADT_ENTRY_HEADER Header;
    uint8_t  BusSource;
    uint8_t  IrqSource;
    uint32_t GlobalSystemInterrupt;
    uint16_t Flags;
} __attribute__((packed)) MADT_ISO;

typedef struct {
    uint32_t ioapic_addr;
    uint32_t gsi_base;
    uint8_t  ioapic_id;
} IOAPIC_INFO;

typedef struct {
    uint8_t irq_source;
    uint32_t gsi;
    uint16_t flags;
} ISO_INFO;

GLOBAL IOAPIC_INFO g_ioapic;
GLOBAL ISO_INFO g_iso[MAX_ISO_ENTRIES];
GLOBAL int g_iso_count;

VOID UEFIParseMADT(ACPI_MADT *Madt);
VOID InitIOAPIC();
VOID LAPICEnable();
USINT32 CalibrateLAPICBusHz();
VOID SetupLapicTimer(USINT32 bus_hz);

#endif