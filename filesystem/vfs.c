#include "vfs.h"
#include "../driver/storage/fat32.h"
#include "../driver/storage/gpt.h"
#include "../driver/storage.h"
#include "../Include/paging.h"
#include "../Include/string.h"
#include "../Include/com.h"
#include <stdint.h>

VFSNode *vfs_root;
FSOperations fat32_ops;

void VFSMountFAT32Root(HBA_PORT *port, VFSNode *mount_point) {
    uintptr_t phys;
    void *virt = palloc_aligned_DMA(512, PAGE_SIZE, &phys);
    if(!ahci_read(port, GPTGlobal->first_lba, 1, (void*)phys)) {
        serial_printf("[Fatal Error] You dont have a PC\n");
        return;
    }

    memcpy(&Fat32Info.boot_sector, virt, sizeof(FAT32BootSector));

    uint32_t reserved = Fat32Info.boot_sector.reserved_sector_count;
    uint32_t fats     = Fat32Info.boot_sector.num_fats;
    uint32_t fsz      = Fat32Info.boot_sector.fat_size_16 ? Fat32Info.boot_sector.fat_size_16 : Fat32Info.boot_sector.fat_size_32;

    Fat32Info.first_data_sector =  GPTGlobal->first_lba + reserved + fats * fsz;
    Fat32Info.sectors_per_cluster = Fat32Info.boot_sector.sectors_per_cluster;
    Fat32Info.root_cluster = Fat32Info.boot_sector.root_cluster;

    serial_printf("[OK] FAT32 Mounted: data_sector=%llu, root_cluster=%u\n", Fat32Info.first_data_sector, Fat32Info.root_cluster);

    mount_point->type = VFS_TYPE_DIR;
    strcpy(mount_point->name, "/");
    mount_point->first_cluster = Fat32Info.root_cluster;
    mount_point->parent = NULL;
    mount_point->children = NULL;
    mount_point->child_count = 0;
    mount_point->fs_ops = &fat32_ops;
    mount_point->fs_data = port;
}

VFSNode* Fat32Lookup(VFSNode *dir, const char *name) {
    uint64_t first_data = Fat32Info.first_data_sector;
    uint32_t spc = Fat32Info.sectors_per_cluster;
    uint32_t bps = Fat32Info.boot_sector.bytes_per_sector;
    uint32_t cluster = dir->first_cluster;
    uint64_t sector = first_data + (cluster - 2) * spc;
    size_t read_size = bps * spc;
    size_t   buf_size  = (read_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    uintptr_t phys;
    void *virt = palloc_aligned_DMA(buf_size, PAGE_SIZE, &phys);
    if(!virt) serial_printf("[Fatal Error] Palloc ERROR\n");

    serial_printf("DEBUG: alloc %u bytes (for %u-byte read)\n", buf_size, read_size);

    if(!ahci_read(dir->fs_data, sector, spc, (void*)phys)) {
        serial_printf("[Error] Failed FAT32LOOKUP\n");
        return NULL;
    }

    int entries = (Fat32Info.boot_sector.bytes_per_sector * spc) / sizeof(DirEntry);
    DirEntry *ent = (DirEntry*)virt;

    LFNEntry *lfn_list[20];
    int lfn_count = 0;

    char lfn_buf[256];

    for(int i = 0; i < entries; i++) {
        DirEntry *e = &ent[i];
        
        if(e->name[0] == 0x00) break;
        if(e->name[0] == 0xE5) continue;

        if(e->attr == 0x0F) {
            LFNEntry *lfn = (LFNEntry*)e;

            if (lfn_count < 20)
                lfn_list[lfn_count++] = lfn;
            continue;
        }

        const char *candidate;
        if(lfn_count > 0) {
            memset(lfn_buf, 0, sizeof(lfn_buf));

            for(int j = lfn_count - 1; j >= 0; j++) {
                LFNEntry *lfn = lfn_list[j];
                int idx = ((lfn->order & 0x1F) - 1) * 13;

                for(int k = 0; k < 5 && lfn->name1[k]; k++) {
                    lfn_buf[idx++] = (char)lfn->name1[k];    
                }
                for(int k = 0; k < 5 && lfn->name2[k]; k++) {
                    lfn_buf[idx++] = (char)lfn->name2[k];    
                }
                for(int k = 0; k < 5 && lfn->name3[k]; k++) {
                    lfn_buf[idx++] = (char)lfn->name3[k];    
                }
                
            }  
            candidate = lfn_buf;
        } else {
            // Build short name 8.3 safely
            char short_name[14];
            int len = 0;
            // base name (up to 8 chars)
            for (; len < 8 && e->name[len] != ' '; len++)
                short_name[len] = e->name[len];
            // extension
            if (e->ext[0] != ' ') {
                short_name[len++] = '.';
                for (int j = 0; j < 3 && e->ext[j] != ' '; j++)
                    short_name[len++] = e->ext[j];
            }
            short_name[len] = '\0';
            candidate = short_name;
        }

        if(strcasecmp(candidate, name) == 0) {
            VFSNode *node = kmalloc(sizeof(VFSNode));
            memset(node, 0, sizeof(VFSNode));
            strcpy(node->name, candidate);
            node->type = (e->attr & 0x10) ? VFS_TYPE_DIR : VFS_TYPE_FILE;
            node->first_cluster = ((uint32_t)e->first_cluster_high << 16) | e->first_cluster_low;
            node->parent = dir;
            node->fs_ops = &fat32_ops;
            node->fs_data = dir->fs_data;

            lfn_count = 0;
            return node;
        }
        lfn_count = 0;
    }

    return NULL;

}
