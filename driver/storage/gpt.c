#include "../storage.h"
#include "gpt.h"
#include "../../Include/paging.h"
#include "../../Include/com.h"
#include "../../Include/string.h"
#include <stdint.h>

GPTPartitionEntry *GPTGlobal;

uint8_t  partition_type_guid[16];   
uint8_t  unique_partition_guid[16];  
uint64_t first_lba;                  
uint64_t last_lba;
uint64_t attributes;
uint16_t partition_name[36];         

void ParseGPT(HBA_PORT *port) {
    
    uintptr_t phys;
    void *virt = palloc_aligned_DMA(512, PAGE_SIZE, &phys);

    int ok = ahci_read(port, 1, 1, (void*)phys);
    if(!ok) {
        serial_printf("[Fatal Error] Your PC has no storage lmao\n");
        return;
    }

    GPTHeader *header = (GPTHeader*)virt;

    if(memcmp(header->signature, "EFI PART", 8) != 0) {
        serial_printf("[Fatal Error] Invalid GPT Signature\n");
        return;
    }

    uint64_t part_entry_lba = header->part_entry_lba;
    uint32_t entry_size = header->size_of_part_entry;
    uint32_t entry_count = header->num_part_entries;

    size_t total_size = entry_count * entry_size;
    size_t total_sector = (total_size + 511) / 512;

    void *entry_virt = palloc_aligned_DMA(total_sector * 512, PAGE_SIZE, &phys);
    if(!ahci_read(port, part_entry_lba, total_sector, (void*)phys)) {
        serial_printf("[Fatal Error] Cannot read GPT entries");
        return;
    }

    GPTGlobal = (GPTPartitionEntry*)entry_virt;

    serial_printf("Partition GUID: ");
    for(int i = 0; i < 16; i++) {
        serial_printf("%x ", GPTGlobal->partition_type_guid[i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) serial_printf("-");
    }
    serial_print("\n");

    
}
