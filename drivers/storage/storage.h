#ifndef _UNOS_STORAGE_INITER_
#define _UNOS_STORAGE_INITER_

#define HBA_PORT_DET_PRESENT   3
#define HBA_PORT_IPM_ACTIVE    1

#define SATA_SIG_ATA    0x00000101  // SATA drive
#define SATA_SIG_ATAPI  0xEB140101  // SATAPI drive
#define SATA_SIG_SEMB   0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM     0x96690101  // Port multiplier
#define AHCI_BASE       0x800000
#define SECTOR_SIZE     512

#include <unostype.h>

void ahci_init(void* abar_addr);
// void init_storage_nvme();

/** In case kalo fallback atau arsitektur storage
 * nya masih menggunakan arsitektur yang lama
 * (for example IDE ATA atau semacamnya)
 */

// void init_ide_atapi();

typedef struct {
    uint32_t clb;       // 0x00, command list base address
    uint32_t clbu;      // 0x04, command list base address upper 32 bits
    uint32_t fb;        // 0x08, FIS base address
    uint32_t fbu;       // 0x0C, FIS base address upper 32 bits
    uint32_t is;        // 0x10, interrupt status
    uint32_t ie;        // 0x14, interrupt enable
    uint32_t cmd;       // 0x18, command and status
    uint32_t reserved0; // 0x1C
    uint32_t tfd;       // 0x20, task file data
    uint32_t sig;       // 0x24, signature
    uint32_t ssts;      // 0x28, SATA status (SCR0)
    uint32_t sctl;      // 0x2C, SATA control (SCR2)
    uint32_t serr;      // 0x30, SATA error (SCR1)
    uint32_t sact;      // 0x34, SATA active (SCR3)
    uint32_t ci;        // 0x38, command issue
    uint32_t sntf;      // 0x3C
    uint32_t fbs;       // 0x40
    uint32_t reserved1[11]; // 0x44 ~ 0x6F
    uint32_t vendor[4];     // 0x70 ~ 0x7F
} __attribute__((packed)) HBA_PORT;

typedef struct {
    uint32_t cap;       // 0x00, Host capability
    uint32_t ghc;       // 0x04, Global host control
    uint32_t is;        // 0x08, Interrupt status
    uint32_t pi;        // 0x0C, Port implemented
    uint32_t vs;        // 0x10, Version
    uint32_t ccc_ctl;   // 0x14, Command completion coalescing control
    uint32_t ccc_pts;   // 0x18, Command completion coalescing ports
    uint32_t em_loc;    // 0x1C, Enclosure management location
    uint32_t em_ctl;    // 0x20, Enclosure management control
    uint32_t cap2;      // 0x24, Host capabilities extended
    uint32_t bohc;      // 0x28, BIOS/OS handoff control and status
    uint8_t  reserved[0xA0 - 0x2C];
    uint8_t  vendor[0x100 - 0xA0];
    HBA_PORT ports[32]; // 1KB dari 0x100 ke depan
} __attribute__((packed)) HBA_MEM;

typedef struct {
    uint8_t  cfl:5;       // Command FIS length in DWORDS, 2 ~ 16
    uint8_t  a:1;         // ATAPI
    uint8_t  w:1;         // Write, 1: write to device, 0: read from device
    uint8_t  p:1;         // Prefetchable

    uint8_t  r:1;         // Reset
    uint8_t  b:1;         // BIST
    uint8_t  c:1;         // Clear busy upon R_OK
    uint8_t  rsv0:1;      // Reserved
    uint8_t  pmp:4;       // Port multiplier port

    uint16_t prdtl;       // Physical region descriptor table length

    volatile uint32_t prdbc; // Physical region descriptor byte count transferred

    uint32_t ctba;        // Command table descriptor base address
    uint32_t ctbau;       // Command table descriptor base address upper 32 bits

    uint32_t rsv1[4];     // Reserved
} __attribute__((packed)) HBA_CMD_HEADER;

typedef struct {
    uint32_t dba;       // Data base address
    uint32_t dbau;      // Data base address upper 32 bits
    uint32_t rsv0;      // Reserved

    uint32_t dbc:22;    // Byte count, 4M max, 0-based
    uint32_t rsv1:9;    // Reserved
    uint32_t i:1;       // Interrupt on completion
} __attribute__((packed)) HBA_PRDT_ENTRY;

typedef struct {
    uint8_t  fis_type;   // FIS_TYPE_REG_H2D = 0x27

    uint8_t  pmport:4;   // Port multiplier
    uint8_t  rsv0:3;     // Reserved
    uint8_t  c:1;        // 1: Command, 0: Control

    uint8_t  command;    // ATA command
    uint8_t  featurel;   // Feature low byte

    uint8_t  lba0;       // LBA low
    uint8_t  lba1;       // LBA mid
    uint8_t  lba2;       // LBA high
    uint8_t  device;     // Device register

    uint8_t  lba3;       // LBA low exp
    uint8_t  lba4;       // LBA mid exp
    uint8_t  lba5;       // LBA high exp
    uint8_t  featureh;   // Feature high byte

    uint8_t  countl;     // Sector count low
    uint8_t  counth;     // Sector count high
    uint8_t  icc;        // Isochronous command completion
    uint8_t  control;    // Control

    uint8_t  rsv1[4];    // Reserved
} __attribute__((packed)) FIS_REG_H2D;

typedef struct {
    uint8_t  cfis[64];             // Command FIS
    uint8_t  acmd[16];             // ATAPI command
    uint8_t  rsv[48];              // Reserved
    HBA_PRDT_ENTRY prdt_entry[1];  // PRDT entries, jumlah tergantung `prdtl`
} __attribute__((packed)) HBA_CMD_TBL;

GLOBAL HBA_PORT *ahci_port;
extern uint64_t abar_global;
int ahci_read(HBA_PORT *port, uint64_t start_lba, uint32_t sector_count, void *buffer);
INT AHCIWriteToStorage(IN HBA_PORT *port, IN USINT64 StartLBA, IN USINT32 SectorCount, OUT VPTR Buffer);

#endif