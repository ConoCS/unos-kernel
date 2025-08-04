#ifndef _FAT32_HEADER_
#define _FAT32_HEADER_

#include <stdint.h>
#include "drivers/storage/storage.h"

typedef struct __attribute__((packed)) {
    uint8_t  jump_boot[3];         // Jump instruction to boot code
    uint8_t  oem_name[8];          // OEM Name (Not important)
    uint16_t bytes_per_sector;     // Bytes per sector
    uint8_t  sectors_per_cluster;  // Sectors per cluster
    uint16_t reserved_sector_count;// Reserved sectors
    uint8_t  num_fats;             // Number of FATs
    uint16_t root_entry_count;     // For FAT12/16, 0 for FAT32
    uint16_t total_sectors_16;     // If zero, check total_sectors_32
    uint8_t  media;                // Media descriptor
    uint16_t fat_size_16;          // FAT size (zero in FAT32)
    uint16_t sectors_per_track;   // For BIOS/CHS booting
    uint16_t num_heads;           // For BIOS/CHS booting
    uint32_t hidden_sectors;      // Hidden sectors before this partition
    uint32_t total_sectors_32;    // Total sectors if total_sectors_16 is zero

    // FAT32 Extended BPB
    uint32_t fat_size_32;         // Sectors per FAT
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;        // Usually 2
    uint16_t fs_info;
    uint16_t backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    uint8_t  volume_label[11];
    uint8_t  fs_type[8];          // Should be "FAT32   "
} FAT32BootSector;

typedef struct {
    uint8_t  name[8];        // Nama file
    uint8_t  ext[3];         // Ekstensi
    uint8_t  attr;           // Atribut (read-only, hidden, system, dir, archive, dll)
    uint8_t  reserved;       // Reserved
    uint8_t  creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high; // Cluster tinggi
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_low;  // Cluster rendah
    uint32_t file_size;
} __attribute__((packed)) DirEntry;

typedef struct __attribute__((packed)) {
    uint8_t  order;               // Urutan LFN (bit 6 = last LFN entry)
    uint16_t name1[5];            // 5 karakter pertama (UTF-16)
    uint8_t  attr;                // Harus 0x0F
    uint8_t  type;                // Selalu 0
    uint8_t  checksum;            // Checksum dari nama short
    uint16_t name2[6];            // 6 karakter berikutnya
    uint16_t zero;                // Selalu 0
    uint16_t name3[2];            // 2 karakter terakhir (total 13 karakter/entry)
} LFNEntry;

typedef struct __attribute__((packed)) {
    uint32_t sector_cluster;
    char LFNFolderName[256];
} SubdirectoryCluster;

typedef struct {
    FAT32BootSector boot_sector;
    uint64_t        first_data_sector;
    uint32_t        sectors_per_cluster;
    uint32_t        root_cluster;
    uint32_t        fat_start_sector;
    HBA_PORT        *fat_port;
} FAT32Info;

extern FAT32Info Fat32Info;
extern SubdirectoryCluster GlobalSC[100];

void ParseFAT32(HBA_PORT *port);
void ReadDirectory(HBA_PORT *port, uint32_t cluster_number);

#endif