#ifndef _GPT_STRUCT_
#define _GPT_STRUCT_

#include <stdint.h>
#include "drivers/storage/storage.h"

#define GPT_HEADER_LBA 1

typedef struct {
    uint8_t  partition_type_guid[16];   // GPT Type GUID
    uint8_t  unique_partition_guid[16];  // Unique partition GUID
    uint64_t first_lba;                  // little‑endian
    uint64_t last_lba;
    uint64_t attributes;
    uint16_t partition_name[36];         // UTF‑16LE (maksimal 36 kod unit = 72 byte)
} __attribute__((packed)) GPTPartitionEntry;

typedef struct {
    char signature[8];              // "EFI PART"
    uint32_t revision;
    uint32_t header_size;
    uint32_t header_crc32;
    uint32_t reserved;
    uint64_t current_lba;
    uint64_t backup_lba;
    uint64_t first_usable_lba;
    uint64_t last_usable_lba;
    uint8_t  disk_guid[16];
    uint64_t part_entry_lba;
    uint32_t num_part_entries;
    uint32_t size_of_part_entry;
    uint32_t part_entry_crc32;
    uint8_t  reserved2[420];
} __attribute__((packed)) GPTHeader;

extern GPTPartitionEntry *GPTGlobal;
void ParseGPT(HBA_PORT *port);

#endif