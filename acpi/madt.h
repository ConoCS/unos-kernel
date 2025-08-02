#ifndef ACPI_MADT_H
#define ACPI_MADT_H

#include <stdint.h>
#include "acpi.h"

#define MAX_ISO_ENTRIES 24
#define IOAPIC_REGSEL(addr) (*(volatile uint32_t*)(addr))
#define IOAPIC_IOWIN(addr)  (*(volatile uint32_t*)((addr) + 0x10))

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

extern IOAPIC_INFO g_ioapic;
extern ISO_INFO g_iso[MAX_ISO_ENTRIES];
extern int g_iso_count;

void UEFIParseMADT(ACPI_MADT *Madt);
void InitIOAPIC();

#endif