#include "vfs.h"
#include "../driver/storage/fat32.h"
#include "../driver/storage/gpt.h"
#include "../driver/storage.h"
#include "../Include/paging.h"
#include "../Include/string.h"
#include "../Include/com.h"
#include <stdint.h>

VFSNode *vfs_root = NULL;
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
    Fat32Info.fat_start_sector =  GPTGlobal->first_lba + reserved;
    Fat32Info.sectors_per_cluster = Fat32Info.boot_sector.sectors_per_cluster;
    Fat32Info.root_cluster = Fat32Info.boot_sector.root_cluster;

    serial_printf("[OK] FAT32 Mounted: data_sector=%llu, root_cluster=%u\n", Fat32Info.first_data_sector, Fat32Info.root_cluster);

    serial_printf("[DEBUG] FAT32 Info: fat_start_sector=%u, first_data_sector=%llu, sectors_per_cluster=%u, root_cluster=%u\n", 
              Fat32Info.fat_start_sector, Fat32Info.first_data_sector, Fat32Info.sectors_per_cluster, Fat32Info.root_cluster);

    mount_point->type = VFS_TYPE_DIR;
    strcpy(mount_point->name, "/");
    mount_point->first_cluster = Fat32Info.root_cluster;
    mount_point->parent = NULL;
    mount_point->children = NULL;
    mount_point->child_count = 0;
    mount_point->fs_ops = &fat32_ops;
    mount_point->fs_data = port;
}

uint32_t fat32_next_cluster(HBA_PORT *port, uint32_t cluster) {
    // 1. Hitung byte-offset dan sektor FAT untuk entri ini
    //serial_printf("Checkpoint 10A <- fat32_next_cluster entry\n");
    uint64_t byte_off = (uint64_t)cluster * 4;
    uint32_t bps      = Fat32Info.boot_sector.bytes_per_sector;
    uint32_t fat_sect = Fat32Info.fat_start_sector + (byte_off / bps);
    uint32_t off_in   = byte_off % bps;

    serial_printf("[DEBUG] Cluster: %u, FAT Sector: %u, Offset in Sector: %u\n", cluster, fat_sect, off_in);

    // 2. Baca sektor FAT ke buffer DMA
    uintptr_t phys2;
    void *virt2 = palloc_aligned_DMA(bps, PAGE_SIZE, &phys2);
    //serial_printf("Checkpoint 10BA kode udah diganti\n");
    if(!ahci_read(port, fat_sect, 1, (void*)phys2)) {
        serial_printf("[E] FAT next-cluster read fail @ sector %u\n", fat_sect);
        pfree_aligned_DMA(virt2, bps);
        //serial_printf("Checkpoint 10B <- fat32_next_cluster exit\n");
        return FAT32_EOC;
    }
    //serial_printf("Checkpoint 10C\n");

    // 3. Ambil 32-bit little endian, mask 28-bit
    uint32_t raw = ((uint32_t*)virt2)[off_in/4] & 0x0FFFFFFF;
    if (raw == cluster) {
        serial_printf("[Error] Self-loop at %u → treating as EOC\n", cluster);
        pfree_aligned_DMA(virt2, bps);
        return FAT32_EOC;
    }
    if (raw < 2 || raw >= FAT32_EOC) {
        //serial_printf("[Error] Out-of-range FAT entry: %u\n", raw);
        pfree_aligned_DMA(virt2, bps);
        return FAT32_EOC;
    }

    serial_printf("[DEBUG] FAT Entry Raw: %u\n", raw);
    serial_printf("[TRACE] Reading Cluster %d, buf virt=0x%x, phys=0x%x\n", cluster, virt2, phys2);

    //serial_printf("Checkpoint 10D\n");
    pfree_aligned_DMA(virt2, bps);
    return raw;
}

VFSNode* Fat32Lookup(VFSNode *dir, const char *name) {
    uint64_t first_data = Fat32Info.first_data_sector;
    uint32_t spc = Fat32Info.sectors_per_cluster;
    uint32_t bps = Fat32Info.boot_sector.bytes_per_sector;
    uint32_t cluster = dir->first_cluster;
    if(cluster < 2) {
        serial_printf("[Error] first cluster invalid: %u\n", cluster);
        return NULL;
    }

    
    size_t read_size = bps * spc;
    size_t   buf_size  = (read_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    uintptr_t phys;
    void *virt = palloc_aligned_DMA(buf_size, PAGE_SIZE, &phys);
    if(!virt) serial_printf("[Fatal Error] Palloc ERROR\n");

    serial_printf("DEBUG: alloc %u bytes (for %u-byte read)\n", buf_size, read_size);
        uint64_t sector = first_data + (cluster - 2) * spc;
        if(!ahci_read(dir->fs_data, sector, spc, (void*)phys)) {
            serial_printf("[Error] Failed FAT32LOOKUP\n");
            pfree_aligned_DMA(virt, buf_size);
            return NULL;
        }

        //serial_printf("This is safe...\n");
        //serial_printf("Checkpoint 1\n");

        int entries = (read_size) / sizeof(DirEntry);
        DirEntry *ent = (DirEntry*)virt;
        //serial_printf("Checkpoint 2\n");

        LFNEntry lfn_list[20];
        int lfn_count = 0;
        //serial_printf("Checkpoint 3\n");

        char lfn_buf[256];
        //serial_printf("Checkpoint 4\n");

        for(int i = 0; i < entries; i++) {
            DirEntry *e = &ent[i];
            
            if(e->name[0] == 0x00) {
                cluster = FAT32_EOC;
                break;
            }
            if(e->name[0] == 0xE5) continue;

            if(e->attr == 0x0F) {
                LFNEntry *lfn = (LFNEntry*)e;

                //serial_printf("RAW LFN[%d]: order=%x, attr=%x\n",  i, lfn->order, e->attr); <- debug dont forgettt.

                uint8_t seq = lfn->order & 0x1F;
                if (seq == 0 || seq > 20) {
                    continue;
                }

                if (lfn_count == 0) memset(lfn_buf, 0, sizeof(lfn_buf));
                //serial_printf("Checkpoint 4A\n");

                if (lfn_count < 20)
                    lfn_list[lfn_count++] = *lfn;
                continue;
            }

            //serial_printf(">> lfn_count = %d\n", lfn_count);
            const char *candidate;
            //serial_printf("Checkpoint 4B\n");
            

            if(lfn_count > 0) {
                //serial_printf("Checkpoint 4C <- LFN Repaired\n");
                int last_idx = 0;
                memset(lfn_buf, 0, sizeof(lfn_buf));

                for(int j = 0; j < lfn_count; j++) {
                    LFNEntry *lfn = &lfn_list[j];
                    uint8_t seq = lfn->order & 0x1F;
                    if (seq == 0 || seq > 20) {
                        continue;
                    }
                    int base = ((lfn->order & 0x1F) - 1) * 13;
                    int idx  = base;

                    if(idx >= (int)sizeof(lfn_buf)) continue;

                // name1
                for (int k = 0; k < 5 && idx < (int)sizeof(lfn_buf)-1; k++) {
                    uint16_t c = lfn->name1[k];
                    if (c == 0x0000 || c == 0xFFFF) break;
                    lfn_buf[idx++] = (char)c;
                }
                // name2
                for (int k = 0; k < 6 && idx < (int)sizeof(lfn_buf)-1; k++) {
                    uint16_t c = lfn->name2[k];
                    if (c == 0x0000 || c == 0xFFFF) break;
                    lfn_buf[idx++] = (char)c;
                }
                // name3
                for (int k = 0; k < 2 && idx < (int)sizeof(lfn_buf)-1; k++) {
                    uint16_t c = lfn->name3[k];
                    if (c == 0x0000 || c == 0xFFFF) break;
                    lfn_buf[idx++] = (char)c;
                }

                    //serial_printf(">> fragment j=%d, order=%x, base=%d\n, j, lfn->order, ((lfn->order & 0x1F) - 1) * 13);
                    //serial_printf(">> lfn_list[%d] @ %p\n", j, (void*)lfn);

                    if (idx > last_idx) last_idx = idx;
                    //serial_printf("Checkpoint 4D\n");
                    
                } 
                
                if(last_idx < (int)sizeof(lfn_buf)) {
                    lfn_buf[last_idx] = '\0';
                } else {
                    lfn_buf[sizeof(lfn_buf)-1] = '\0';
                }
            
                //serial_printf("Checkpoint 4H\n");
                serial_printf("LFN terminated at %d: '%s'\n", last_idx, lfn_buf);
                candidate = lfn_buf;
                lfn_count = 0;
                //serial_printf("candidate: %s", candidate);
                //serial_printf("Checkpoint 5\n");
                //serial_printf("Gatau\n");
            } else {
                // Build short name 8.3 safely
                char short_name[14];
                int len = 0;
                //serial_printf("Checkpoint 5A\n");
                // base name (up to 8 chars)
                for (; len < 8 && e->name[len] != ' '; len++)
                    short_name[len] = e->name[len];
                    //serial_printf("Checkpoint 5B\n");
                // extension
                if (e->ext[0] != ' ') {
                    short_name[len++] = '.';
                    for (int j = 0; j < 3 && e->ext[j] != ' '; j++)
                        short_name[len++] = e->ext[j];
                }
                short_name[len] = '\0';
                candidate = short_name;
                //serial_printf("Checkpoint 5C\n");
            }

            //serial_printf("Checkpoint 6\n");
            //serial_printf("CMP: '%s' vs '%s'\n", candidate, name);
            if(strcasecmp(candidate, name) == 0) {
                //serial_printf("Checkpoint 6A <- Masuk CMP\n");
                VFSNode *node = kmalloc(sizeof(VFSNode));
                if(!node) {
                    serial_printf("[Error] Fat32Lookup: kmalloc failed\n");
                    pfree_aligned_DMA(virt, buf_size);
                    return NULL;
                }
                serial_printf("Pointer kmalloc node Lookup: %p\n", node);
                memset(node, 0, sizeof(VFSNode));
                //serial_printf("Checkpoint 6B\n");
                strcpy(node->name, candidate);
                node->type = (e->attr & 0x10) ? VFS_TYPE_DIR : VFS_TYPE_FILE;
                if((e->attr & 0x10) == 0) {
                    node->size = e->file_size;
                }
                //serial_printf("Checkpoint 6C\n");
                node->first_cluster = ((uint32_t)e->first_cluster_high << 16) | e->first_cluster_low;
                node->parent = dir;
                node->fs_ops = &fat32_ops;
                node->fs_data = dir->fs_data;
                //serial_printf("Checkpoint 6D <- Akhir CMP\n");

                lfn_count = 0;
                return node;
            }
            //serial_printf("Checkpoint 7\n");
            lfn_count = 0;
        }
    pfree_aligned_DMA(virt, buf_size);

    return NULL;

}

void* Fat32Open(VFSNode *cwd, const char *path) {
    VFSNode *node;

    if(path[0] == '/') {
        node = vfs_root;
        while (*path == '/') path++;
    } else {
        node = cwd;
    }

    char seg[256];
    while(node && *path) {
        char *p = seg;
        while(*path && *path != '/') {
            *p++ = *path++;
        }
        *p = '\0';

        while(*path == '/') path++;

        node = Fat32Lookup(node, seg);
    }
    return node;
}

int Fat32Read(VFSNode *node, size_t offset, void* buffer, size_t size) {
    if (!node || node->type != VFS_TYPE_FILE || size == 0) return -1;

    uint32_t cluster = node->first_cluster;
    uint32_t bps = Fat32Info.boot_sector.bytes_per_sector;
    uint32_t spc = Fat32Info.sectors_per_cluster;
    uint64_t first_data_sector = Fat32Info.first_data_sector;

    size_t filesize = node->size;
    if (offset >= filesize) return 0;

    size_t remaining = (offset + size > filesize) ? (filesize - offset) : size;
    size_t total_read = 0;
    uint8_t *user_buf = (uint8_t*)buffer;

    size_t bytes_per_cluster = bps * spc;
    while(offset >= bytes_per_cluster) {
        cluster = fat32_next_cluster((HBA_PORT*)node->fs_data, cluster);
        if (cluster >= FAT32_EOC) return total_read;
        offset -= bytes_per_cluster;
    }

    #define MAX_CLUSTERS_CHAIN 4096
    int chain_count= 0;

    serial_printf("[ENTER] cluster=%u, remaining=%u\n", cluster, remaining);

    while(remaining > 0 && cluster < FAT32_EOC) {
        if (++chain_count > MAX_CLUSTERS_CHAIN) {
            serial_printf("[Error] Fat32Read: Too many clusters\n");
            break;
        }

        uint64_t sector = first_data_sector + (cluster - 2) * spc;

        size_t read_offset = offset;
        size_t read_len = (remaining < (bytes_per_cluster - read_offset)) ? remaining : (bytes_per_cluster - read_offset);

        serial_printf("[DEBUG] Reading cluster: %u, sector: %llu\n", cluster, sector);
        serial_printf("[DEBUG] Read offset: %llu, Read length: %llu\n", read_offset, read_len);

        uintptr_t phys;
        void *dma_buf = palloc_aligned_DMA(spc * bps, PAGE_SIZE, &phys);
        if (dma_buf) {
        }
        //serial_printf(" Checkpoint custom AC\n");
       if (!dma_buf) {
            serial_printf("[ERROR] DMA alloc fail for cluster %u\n", cluster);
            break;
        }
        //serial_printf(" Checkpoint custom AB\n");
        if(!ahci_read(node->fs_data, sector, spc, (void*)phys)) {
            serial_printf("[Error] Fat32Read: Failed read cluster %u (sector %llu)\n", cluster, sector);
            pfree_aligned_DMA(dma_buf, spc * bps);
            break;
        }
        //serial_printf(" Checkpoint custom AA\n");


        //serial_printf("[MEMCPY DBG] total_read=%llu, remaining=%llu\n", total_read, remaining);
        //serial_printf("  -> user_buf = %p\n", user_buf);
        //serial_printf("  -> dst = %p\n", user_buf + total_read);
        //serial_printf("  -> dma_buf = %p (phys=0x%llx)\n", dma_buf, phys);
        //serial_printf("  -> read_offset=%zu, read_len=%zu\n", read_offset, read_len);
        memcpy(user_buf + total_read, (uint8_t*)dma_buf + read_offset, read_len);

        pfree_aligned_DMA(dma_buf, spc * bps);

        total_read += read_len;
        remaining -= read_len;
        offset = 0;

        uint32_t prev_cluster = cluster;

        serial_printf("[LOOP] done cluster %u → total_read=%u, remaining=%u\n",
              cluster, total_read, remaining);
        cluster = fat32_next_cluster((HBA_PORT*)node->fs_data, cluster);

        //serial_printf("[CLUSTER DEBUG] prev=%u -> next=%u, total_read=%u bytes, remaining=%u bytes\n",
                    //prev_cluster, cluster, total_read, remaining);

        if (cluster < 2 || cluster >= FAT32_EOC)
            break;

    }

    return total_read;
}


