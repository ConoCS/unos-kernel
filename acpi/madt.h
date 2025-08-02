#ifndef ACPI_MADT_H
#define ACPI_MADT_H

#include <stdint.h>
#include "acpi.h"

typedef struct {
    ACPI_HEADER Header;
    uint32_t LocalApicAddress;
    uint32_t Flags;
    uint8_t Entries[];
} __attribute__((packed)) ACPI_MADT;

typedef struct {
    uint8_t Type;
    uint8_t Length;
    uint8_t AcpiProcessorId;
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

void UEFIParseMADT(ACPI_MADT *Madt);

#endif