#include <unoskrnl.h>
#include <kstring.h>

GPTPartitionEntry *GPTGlobal;       

void PrintGUID(uint8_t *guid) {
    serial_printf("%02X%02X%02X%02X-", guid[3], guid[2], guid[1], guid[0]);
    serial_printf("%02X%02X-",       guid[5], guid[4]);
    serial_printf("%02X%02X-",       guid[7], guid[6]);
    serial_printf("%02X%02X-",       guid[8], guid[9]);
    for (int i = 10; i < 16; i++)
        serial_printf("%02X", guid[i]);
}

void ParseGPT(HBA_PORT *port) {
    if (!ahci_port) {
        serial_printf("[ERROR] AHCI port is NULL, cannot parse GPT\n");
        while(1) asm("hlt");
    }

    
    uintptr_t phys;
    void *virt = palloc_aligned_DMA(512, PAGE_SIZE, &phys);

    int ok = ahci_read(port, GPT_HEADER_LBA, 1, (void*)phys);
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

    GPTPartitionEntry *entries = (GPTPartitionEntry*)entry_virt;
    GPTGlobal = &entries[0];


    serial_printf("Partition GUID: ");
    PrintGUID(GPTGlobal->partition_type_guid);
    serial_print("\n");
    serial_print("\n");
    serial_printf("First Partition Start LBA: %llu\n", GPTGlobal->first_lba);
    serial_printf("Last Partition End LBA: %llu\n", GPTGlobal->last_lba);
    serial_printf("Attributes: %llu\n", GPTGlobal->attributes);
    serial_printf("Partition Name: ");
    for(INT i = 0; i < 36; i++) {
        USINT16 ch = GPTGlobal->partition_name[i];
        if(ch == 0) break;
        serial_printf("%c", (char)ch);
    }
    serial_printf("Partition Raw UTF16:\n");
    for(int i = 0; i < 36; i++) {
        serial_printf("%04x ", GPTGlobal->partition_name[i]);
    }
    serial_print("\n");
    serial_print("\n");
 
}
