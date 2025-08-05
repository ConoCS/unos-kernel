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
#define MAX_CPU_COUNT 256

#define LAPIC_SVR   0xF0
#define LAPIC_EOI   0xB0

#define APIC_BASE        0xFEE00000
#define APIC_ICR_LOW     0x300   // Interrupt Command Register Low
#define APIC_ICR_HIGH    0x310   // Interrupt Command Register High

#define ICR_DELIVERY_MODE_FIXED     (0 << 8)
#define ICR_DELIVERY_MODE_INIT      (5 << 8)
#define ICR_DELIVERY_MODE_STARTUP   (6 << 8)
#define ICR_LEVEL_ASSERT            (1 << 14)
#define ICR_DEST_MODE_PHYSICAL      0
#define ICR_DEST_SHORTHAND_NONE    (0 << 18)
#define TRAMPOLINE_ADDR 0x7000


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

typedef struct {
    USINT8 processor_id;
    USINT8 apic_id;
    BOOL is_bsp; // true untuk core pertama
    BOOL started;
} CPU_CORE;

GLOBAL IOAPIC_INFO g_ioapic;
GLOBAL ISO_INFO g_iso[MAX_ISO_ENTRIES];
GLOBAL int g_iso_count;
GLOBAL CPU_CORE g_cpu_cores[MAX_CPU_COUNT];
GLOBAL USINT32 g_cpu_count;


VOID UEFIParseMADT(ACPI_MADT *Madt);
VOID InitIOAPIC();
VOID LAPICEnable();
USINT32 CalibrateLAPICBusHz();
VOID SetupLapicTimer(USINT32 bus_hz);

#endif