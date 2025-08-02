#ifndef ACPI_H
#define ACPI_H

#include <stdint.h>

#define ACPI_MADT_SIGNATURE  0x43495041  // 'APIC' dalam little-endian


typedef struct {
    char     Signature[4];     // 4-char ASCII ID, misalnya "APIC", "FACP"
    uint32_t Length;           // Total size of the table termasuk header
    uint8_t  Revision;         // Revisi tabel ini (versi)
    uint8_t  Checksum;         // Seluruh tabel harus checksum = 0
    char     OemId[6];         // ID pabrik pembuat (OEM)
    char     OemTableId[8];    // ID tabel unik buatan OEM
    uint32_t OemRevision;      // Versi revisi OEM
    uint32_t CreatorId;        // Siapa yang bikin tabel ini
    uint32_t CreatorRevision;  // Versi dari tool pembuatnya
} __attribute__((packed)) ACPI_HEADER;

extern uint32_t IoAPICAddress;

void init_acpi();
void apic_send_eoi();


#endif // ACPI_H
