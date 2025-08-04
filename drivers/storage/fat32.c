#include <unoskrnl.h>

SubdirectoryCluster GlobalSC[100];

FAT32Info Fat32Info;

void ParseFAT32(HBA_PORT *port) {
    uintptr_t phys;
    void *virt = palloc_aligned_DMA(512, PAGE_SIZE, &phys);

    int ok = ahci_read(port, GPTGlobal->first_lba, 1, (void*)phys);
    if(!ok) {
        serial_printf("[Fatal error] Failed to ahci_read GPTPartitionEntry\n");
    }

    memcpy(&Fat32Info.boot_sector, virt, sizeof(FAT32BootSector));

    FAT32BootSector *bs = (FAT32BootSector*)virt;

    if(bs->bytes_per_sector == 0 || bs->sectors_per_cluster == 0) {
        serial_printf("[Fatal Error] Invalid FAT32 Boot Sector\n");
        return;
    }

    uint32_t bytes_per_sector = bs->bytes_per_sector;
    uint32_t sector_per_cluster = bs->sectors_per_cluster;
    uint32_t reserved_sectors = bs->reserved_sector_count;
    uint32_t fat = bs->num_fats;
    uint32_t fat_size = bs->fat_size_16 ? bs->fat_size_16 : bs->fat_size_32;

    uint64_t first_data_sector = GPTGlobal->first_lba + reserved_sectors + (fat * fat_size);

    uint64_t root_sector = first_data_sector + (bs->root_cluster - 2) * sector_per_cluster;
    HBA_PORT *fat_port = (HBA_PORT*)port;

    ok = ahci_read(port, root_sector, sector_per_cluster, (void*)phys);
    if (!ok) {
        serial_printf("[Fatal Error] Can't read root sector\n");
    }

    uint8_t *dir = virt;
    int entries = (bytes_per_sector * sector_per_cluster) / sizeof(DirEntry);
    DirEntry *entry_list = (DirEntry*)dir;

    char lfn_buf[256];
    int lfn_active = 0;
    lfn_buf[0] = '\0';

    for(int i = 0; i < entries; i++) {
        DirEntry *ent = &entry_list[i];

        if(ent->name[0] == 0x00) break;
        if(ent->name[0] == 0xE5) continue;

        if(ent->attr == 0x0F) {
            LFNEntry *lfn = (LFNEntry*)ent;
            int ord = (lfn->order & 0x1F) - 1;
            int offset = ord * 13;
            int idx = 0;

            // name1: 5 karakter
            for (int j = 0; j < 5; j++) {
                if (lfn->name1[j] == 0xFFFF || lfn->name1[j] == 0x0000) break;
                lfn_buf[offset + idx++] = (char)lfn->name1[j];
            }

            // name2: 6 karakter
            for (int j = 0; j < 6; j++) {
                if (lfn->name2[j] == 0xFFFF || lfn->name2[j] == 0x0000) break;
                lfn_buf[offset + idx++] = (char)lfn->name2[j];
            }

            // name3: 2 karakter
            for (int j = 0; j < 2; j++) {
                if (lfn->name3[j] == 0xFFFF || lfn->name3[j] == 0x0000) break;
                lfn_buf[offset + idx++] = (char)lfn->name3[j];
            }

            lfn_buf[offset + idx] = '\0';
            lfn_active = 1;
            continue;
        } else { if (ent->name[0] == 0x00 || ent->name[0] == 0xE5) continue; }

        char name[13];
        memcpy(name, ent->name, 8);
        name[8] = '.';
        memcpy(name + 9, ent->ext, 3);
        name[12] = '\0';

        for (int j = 7; j >= 0 && name[j] == ' '; j--) name[j] = '\0';
        for (int j = 11; j >= 9 && name[j] == ' '; j--) name[j] = '\0';

        uint32_t first_cluster = (ent->first_cluster_high << 16) | ent->first_cluster_low;

        if (lfn_active) {
            // Ada LFN
            serial_printf("%s  %s  cluster=%u  size=%u\n",
                lfn_buf,
                (ent->attr & 0x10) ? "<DIR>" : "<FILE>",
                first_cluster,
                ent->file_size);
            lfn_buf[0] = '\0';  // Reset setelah dipakai
            lfn_active = 0;
        } else {
            // Ga ada LFN, fallback ke 8.3
            serial_printf("%s  %s  cluster=%u  size=%u\n",
                name,
                (ent->attr & 0x10) ? "<DIR>" : "<FILE>",
                first_cluster,
                ent->file_size);
        }   
    }
}

void ReadDirectory(HBA_PORT *port, uint32_t cluster_number) {
    uintptr_t phys;
    void *virt = palloc_aligned_DMA(512, PAGE_SIZE, &phys);

    int ok = ahci_read(port, GPTGlobal->first_lba, 1, (void*)phys);

    FAT32BootSector *bs = (FAT32BootSector*)virt;

    uint32_t bytes_per_sector = bs->bytes_per_sector;
    uint32_t sector_per_cluster = bs->sectors_per_cluster;
    uint32_t reserved_sectors = bs->reserved_sector_count;
    uint32_t fat = bs->num_fats;
    uint32_t fat_size = bs->fat_size_16 ? bs->fat_size_16 : bs->fat_size_32;

    uint64_t first_data_sector = GPTGlobal->first_lba + reserved_sectors + (fat * fat_size);

    uint64_t folder_sector = first_data_sector + (cluster_number - 2) * sector_per_cluster;
    if (folder_sector < 2) {
        serial_printf("[Error] Cluster number invalid: %d. Must be >= 2\n", cluster_number);
        return;
    }

    ok = ahci_read(port, folder_sector, sector_per_cluster, (void*)phys);

    uint8_t *dir = virt;
    int entries = (bytes_per_sector * sector_per_cluster) / sizeof(DirEntry);
    DirEntry *entry_list = (DirEntry*)dir;

    char lfn_buf[256];
    int lfn_active = 0;
    lfn_buf[0] = '\0';

    for(int i = 0; i < entries; i++) {
        DirEntry *ent = &entry_list[i];

        if(ent->name[0] == 0x00) break;
        if(ent->name[0] == 0xE5) continue;

        if(ent->attr == 0x0F) {
            LFNEntry *lfn = (LFNEntry*)ent;
            int ord = (lfn->order & 0x1F) - 1;
            int offset = ord * 13;
            int idx = 0;

            // name1: 5 karakter
            for (int j = 0; j < 5; j++) {
                if (lfn->name1[j] == 0xFFFF || lfn->name1[j] == 0x0000) break;
                lfn_buf[offset + idx++] = (char)lfn->name1[j];
            }

            // name2: 6 karakter
            for (int j = 0; j < 6; j++) {
                if (lfn->name2[j] == 0xFFFF || lfn->name2[j] == 0x0000) break;
                lfn_buf[offset + idx++] = (char)lfn->name2[j];
            }

            // name3: 2 karakter
            for (int j = 0; j < 2; j++) {
                if (lfn->name3[j] == 0xFFFF || lfn->name3[j] == 0x0000) break;
                lfn_buf[offset + idx++] = (char)lfn->name3[j];
            }

            lfn_buf[offset + idx] = '\0';
            lfn_active = 1;
            continue;
        } else { if (ent->name[0] == 0x00 || ent->name[0] == 0xE5) continue; }

        char name[13];
        memcpy(name, ent->name, 8);
        name[8] = '.';
        memcpy(name + 9, ent->ext, 3);
        name[12] = '\0';

        for (int j = 7; j >= 0 && name[j] == ' '; j--) name[j] = '\0';
        for (int j = 11; j >= 9 && name[j] == ' '; j--) name[j] = '\0';

        uint32_t first_cluster = (ent->first_cluster_high << 16) | ent->first_cluster_low;

        if (lfn_active) {
            // Ada LFN
            serial_printf("%s  %s  cluster=%u  size=%u\n",
                lfn_buf,
                (ent->attr & 0x10) ? "<DIR>" : "<FILE>",
                first_cluster,
                ent->file_size);
            lfn_buf[0] = '\0';  // Reset setelah dipakai
            lfn_active = 0;
        } else {
            // Ga ada LFN, fallback ke 8.3
            serial_printf("%s  %s  cluster=%u  size=%u\n",
                name,
                (ent->attr & 0x10) ? "<DIR>" : "<FILE>",
                first_cluster,
                ent->file_size);
        }   
    }
}


