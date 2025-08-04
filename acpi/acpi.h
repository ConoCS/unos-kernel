#ifndef ACPI_H
#define ACPI_H

#include <unostype.h>

#define ACPI_MADT_SIGNATURE  0x43495041  // 'APIC' dalam little-endian


typedef struct {
    CONST char     Signature[4];         // 4-char ASCII ID, misalnya "APIC", "FACP"
    USINT32         Length;               // Total size of the table termasuk header
    USINT8          Revision;             // Revisi tabel ini (versi)
    USINT8          Checksum;             // Seluruh tabel harus checksum = 0
    CONST char     OemId[6];             // ID pabrik pembuat (OEM)
    CONST char     OemTableId[8];        // ID tabel unik buatan OEM
    USINT32         OemRevision;          // Versi revisi OEM
    USINT32         CreatorId;            // Siapa yang bikin tabel ini
    USINT32         CreatorRevision;      // Versi dari tool pembuatnya
} __attribute__((packed)) ACPI_HEADER;

GLOBAL USINT32 IoAPICAddress;

VOID init_acpi();
VOID apic_send_eoi();


#endif // ACPI_H
