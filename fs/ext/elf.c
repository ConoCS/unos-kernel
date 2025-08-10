/**
 *  Hak Cipta Dilindungi (c) 2025 UnOS Team of ConoCS
 *
 *  Lisensi: GPL v3.0
 *  Semua kode dalam file ini dan modul kernel terkait harus bersifat
 *  FREE dan OPEN SOURCE, sesuai dengan lisensi tersebut.
 *
 *  Nama File:
 *      elf.c
 *
 *  Ringkasan:
 *      elf related function
 *
 *  Penulis:
 *      Rasya, 09-Aug-2025
 *
 *  Penafian:
 *      Penulis bertanggung jawab apabila modul ini menyebabkan
 *      kerusakan, kehilangan, atau ketidakstabilan pada kernel.
 *
 *  Histori Revisi:
 *      Belum ada revisi.
 */

 /**
  *  Copyright (c) 2025 UnOS Team of ConoCS
  *
  *  License: GPL v3.0
  *  All code in this file and related kernel modules must remain
  *  FREE and OPEN SOURCE, in accordance with this license.
  *
  *  File Name:
  *      elf.c
  *
  *  Summary:
  *      elf related function
  *
  *  Author:
  *      Rasya, 09-Aug-2025
  *
  *  Disclaimer:
  *      The author is not responsible for any damage, data loss,
  *      or instability in the kernel caused by this module.
  *
  *  Revision History:
  *      Sunday 10 August 2025: Adding map_virtual_userland to ElfLoad64
  */

#define UNOS_ELF_USERLAND_MACRO
#include <unoskrnl.h>

#define PF_X 0x1  // Execute permission
#define PF_W 0x2  // Write permission
#define PF_R 0x4  // Read permission


UNSTATUS
ELFLOAD
Elf64Load(
    IN VPTR elfData
){
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elfData;

    if(ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' ||
        ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
            serial_printf("[ELF64] Failed to open: Not an ELF file\n");
            return STATUS_FAIL;
    }

    Elf64_Phdr *phdr = (Elf64_Phdr*)((USINT8*)elfData + ehdr->e_phoff);

    for(UINT i = 0; i < ehdr->e_phnum; i++) {
        if(phdr[i].p_type == PT_LOAD) {
            USINT64 segment_size = phdr[i].p_memsz;
            USINT64 file_size = phdr[i].p_filesz;
            USINT64 virtual_addr = phdr[i].p_vaddr;
            USINT64 offset_in_file = phdr[i].p_offset;

            // align virtual address down and size up to page size
            USINT64 vaddr_page = virtual_addr & ~(PAGE_SIZE - 1);
            USINT64 size_page = ((virtual_addr + segment_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)) - vaddr_page;

            // alok dan mapping halaman physical nya sebanyak segment_size
            for(USINT64 off = 0; off < size_page; off += PAGE_SIZE) {
                void* phys_page = palloc_page();
                memset(phys_page, 0, PAGE_SIZE);

                uint64_t page_flags = 0;
                page_flags = PAGE_PRESENT | PAGE_USER | PAGE_WRITABLE;  // always present & user
                if (phdr[i].p_flags & PF_W) page_flags |= PAGE_WRITABLE;
                Printk(KINFO, "Before clear NX: page_flags = 0x%x\n", page_flags);
                if (phdr[i].p_flags & PF_X) {
                    page_flags &= ~PAGE_NX;
                    Printk(KINFO, "After clear NX: page_flags = 0x%x\n", page_flags);
                } else {
                    page_flags |= PAGE_NX;
                    Printk(KINFO, "After set NX: page_flags = 0x%x\n", page_flags);
                }
                Printk(KINFO, "PAGE_NX = 0x%x\n", (unsigned long long)PAGE_NX);

                Printk(KINFO, "Error that will hapenn next: map_virtual_user_advanced\n");
                Printk(KINFO, "PF macros: PRESENT=0x%x WRIT=0x%x USER=0x%x NX=0x%x\n",
                    (unsigned long long)PAGE_PRESENT,
                    (unsigned long long)PAGE_WRITABLE,
                    (unsigned long long)PAGE_USER,
                    (unsigned long long)PAGE_NX);
                Printk(KINFO, "page_flags computed = 0x%x\n", (unsigned long long)page_flags);
                map_virtual_user_advanced(vaddr_page + off, (USINT64)phys_page, PAGE_SIZE, page_flags);
                Printk(KINFO,"Error not in map_virtual_user_advanced\n");
            }

            Printk(KINFO, "Or maybe in here?\n");
            Printk(KINFO, "vaddr_page value: %llu, virtual_addr value: %llu", vaddr_page, virtual_addr);
            memcpy((VOID*)(vaddr_page), (USINT8*)elfData + offset_in_file, file_size);
            Printk(KINFO, "I guess not\n");
        }
    }
    return STATUS_OK;
}

UNFUNCTION
UnGoToUserland(IN CONST CHARA8 *path) {
    VFSNode *elf = Fat32Open(vfs_root, path);
    if(!elf || elf->type != VFS_TYPE_FILE) {
        Printk(KERR, "UnGoToUserland: Not a type of Userland app\n");
        Panic(FAILED_INITIALIZATION_USERLAND);
    }

    Printk(KINFO, "Debug: File size: %llu bytes", elf->size);

    size_t file_size = elf->size;
    void *buffer = kmalloc(file_size);
    if(!buffer) {
        Printk(KERR, "UnGoToUserland: Error malloc\n");
        Panic(FAILED_INITIALIZATION_USERLAND);
    }

    Printk(KINFO, "UnGoToUserland: Buffer allocated at %p\n", buffer);

    int read_bytes = Fat32Read(elf, 0, buffer, file_size);
    if(!read_bytes || (size_t)read_bytes != file_size){
        Printk(KERR, "UnGoToUserland: Error reading an Userland file\n");
        Panic(FAILED_INITIALIZATION_USERLAND);
    }

    Printk(KSUCCESS, "UnGoToUserland: Succesfully init Userland\n");

    Elf64Load(buffer);

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)buffer;
    UINTN entry_point = (UINTN)ehdr->e_entry;

    UINTN user_stack_top = USERLAND_STACK_TOP;

    serial_printf("Checkpoint A\n");

    UnStartUserland((VPTR)entry_point, (VPTR)user_stack_top);
}



