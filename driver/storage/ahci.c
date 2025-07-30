#include <stdint.h>
#include "../../Include/com.h"
#include "../storage.h"
#include "../../Include/string.h"
#include "../../Include/paging.h"
#include <stdbool.h>

/* Ini utnuk fungsi AHCI_STOP_CMD */
#define HBA_PxCMD_ST   (1 << 0)   // Start
#define HBA_PxCMD_FRE  (1 << 4)   // FIS Receive Enable
#define HBA_PxCMD_FR   (1 << 14)  // FIS Receive Running (RO)
#define HBA_PxCMD_CR   (1 << 15)  // Command List Running (RO)

volatile HBA_MEM* abar;
HBA_PORT *ahci_port = NULL;
uint64_t abar_global;

HBA_PORT *ahci_probe_port();
void ahci_port_rebase(HBA_PORT *port, int portno);

void ahci_stop_cmd(HBA_PORT *port) {
    // Clear ST (stop command engine)
    port->cmd &= ~HBA_PxCMD_ST;

    // Clear FRE (stop FIS receive engine)
    port->cmd &= ~HBA_PxCMD_FRE;

    // Tunggu sampai hardware berhenti beneran
    while (port->cmd & (HBA_PxCMD_FR | HBA_PxCMD_CR));
}

void ahci_start_cmd(HBA_PORT *port) {
    // Enable FIS receive
    port->cmd |= HBA_PxCMD_FRE;

    // Enable command processing
    port->cmd |= HBA_PxCMD_ST;
}

int find_cmdslot(HBA_PORT *Port) {
    uint32_t slots = (Port->sact | Port->ci);
    for (int i = 0; i < 32; i++) {
        if ((slots & (1 << i)) == 0) {
            return i;
        }
    } return -1;
}

void ahci_init(void* abar_addr){
    abar = (volatile HBA_MEM*)abar_addr;

    serial_printf("AHCI Init ABAR at %p\n", (void*)abar);
    abar_global = (uint64_t)(uintptr_t)abar;
    serial_printf("Version: %X\n", abar->vs);
    serial_printf("Ports Implemented: %X\n\n", abar->pi);

    for (int i = 0; i < 32; i++){
        if (abar->pi & (1 << i)) {
            HBA_PORT* port = &abar->ports[i];
            uint32_t ssts = port->ssts;
            uint8_t ipm = (ssts >> 8) & 0x0F;
            uint8_t det = ssts & 0x0F;

            if(det == HBA_PORT_DET_PRESENT && ipm == HBA_PORT_IPM_ACTIVE) {
                serial_printf("-> Port %d: Device present and active. Signature: %X\n", i, port->sig);
            } else {
                serial_printf("-> Port %d: No active device\n", i);
            }
        }
    }
    ahci_probe_port();
    
}

HBA_PORT *ahci_probe_port() {
    
    uint32_t pi = abar->pi;

    for(int i = 0; i < 32; i++) {
        if(pi & (1 << i)) {
            HBA_PORT *port = &abar->ports[i];
            uint32_t ssts = port->ssts;
            uint8_t ipm = (ssts >> 8) & 0x0F;
            uint8_t det = ssts & 0x0F;

            if(det == HBA_PORT_DET_PRESENT && ipm == HBA_PORT_IPM_ACTIVE) {
                switch(port->sig) {
                    case SATA_SIG_ATAPI:
                    serial_printf("Port %d: SATAPI device\n", i);
                    ahci_port_rebase(port, i);
                    ahci_port = port;
                    break;
                    case SATA_SIG_SEMB:
                    serial_printf("Port %d: SEMB device\n", i);
                    ahci_port_rebase(port, i);
                    ahci_port = port;
                    break;
                case SATA_SIG_PM:
                    serial_printf("Port %d: Port Multiplier\n", i);
                    ahci_port_rebase(port, i);
                    ahci_port = port;
                    break;
                case SATA_SIG_ATA:
                    serial_printf("Port %d: SATA drive detected!\n", i);
                    ahci_port_rebase(port, i);
                    ahci_port = port;
                    return port;
                default:
                    serial_printf("Port %d: Unknown device. Signature: %X\n", i, port->sig);
                    break;
                }
            }
        }
    }
    serial_printf("No usable SATA port found\n");
    return 0;
}

void ahci_port_rebase(HBA_PORT *port, int portno) {
    ahci_stop_cmd(port);

    port->clb = AHCI_BASE + (portno << 10);
    port->clbu = 0;
    memset((void*)(uintptr_t)port->clb, 0, 1024);

    port->fb = AHCI_BASE + (32 << 10) + (portno << 8);
    port->fbu = 0;
    memset((void*)(uintptr_t)port->fb, 0, 256);
    
    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)(uintptr_t)port->clb;
    for(int i = 0; i < 32; i++) {
        cmdheader[i].prdtl = 8;
        uint64_t ctba = AHCI_BASE + (40 << 10) + (portno << 13) + (i << 8);
        cmdheader[i].ctba = (uint32_t)ctba;
        cmdheader[i].ctbau = (uint32_t)(ctba >> 32);
        memset((void*)(uintptr_t)ctba, 0, 256);
    }

    ahci_start_cmd(port);
    serial_printf("AHCI Port %d has been rebased \n", portno);
}

int ahci_read(HBA_PORT *port, uint64_t start_lba, uint32_t sector_count, void *buffer) {
    // 0. clear interrupt status
    port->is = (uint32_t)-1;

    /* 1. Cari Slot Kosong */
    int slot = find_cmdslot(port);
    if (slot == -1) {
        serial_printf("AHCI_READ: There is no empty slot here...\n");
        return 0;
    }

    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)(uintptr_t)(port->clb);
    cmdheader += slot;
    cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
    cmdheader->w = 0;
    cmdheader->prdtl = 1;

    /* 2. Setup Command table */
    HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(uintptr_t)(cmdheader->ctba);
    memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) + (cmdheader->prdtl - 1) * sizeof(HBA_PRDT_ENTRY));

    /* 3. Setup PRDT Entry */
    cmdtbl->prdt_entry[0].dba = (uint32_t)buffer;
    cmdtbl->prdt_entry[0].dbau = 0;
    cmdtbl->prdt_entry[0].dbc = (sector_count * 512) - 1;
    cmdtbl->prdt_entry[0].i = 1;

    /* 4. Setup FIS */
    FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);
    cmdfis->fis_type = 0x27;
    cmdfis->c = 1;
    cmdfis->command = 0x25;

    cmdfis->lba0 = (uint8_t)start_lba;
    cmdfis->lba1 = (uint8_t)(start_lba >> 8);
    cmdfis->lba2 = (uint8_t)(start_lba >> 16);
    cmdfis->device = 1 << 6;

    cmdfis->lba3 = (uint8_t)(start_lba >> 24);
    cmdfis->lba4 = (uint8_t)(start_lba >> 32);
    cmdfis->lba5 = (uint8_t)(start_lba >> 40);

    cmdfis->countl = sector_count & 0xFF;
    cmdfis->counth = (sector_count >> 8) & 0xFF;
    

    /* 5. Tunggu sampai device siap */
    uint32_t spin = 0;
    const uint32_t TIMEOUT = 1000000;
    while((port->tfd & 0x80) && spin++ < TIMEOUT);
    if (spin >= TIMEOUT) {
        serial_printf("[Error] AHCI_READ: Timeout waiting for device ready\n");
        return 0;
    } 

    /* 6. Start command */
    port->ci |= 1 << slot;

    /* 7. Tunggu selesai */
    while(1) {
        if ((port->ci & (1 << slot)) == 0) break;
        if (port->is & (1 << 30)) return 0;
    }

    /* 8. Cek ERROR */
    if(port->is & (1 << 30)) return 0;
    port->is = (uint32_t)-1;

    serial_printf("AHCI_READ: Read LBA %llu (%u sectors) done\n", start_lba, sector_count);
    return 1; // berhasil
}