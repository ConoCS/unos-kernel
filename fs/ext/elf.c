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
  *      No revisions yet.
  */

#define UNOS_ELF_USERLAND_MACRO
#include <unoskrnl.h>

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
            VPTR dest = (VPTR)(UINTN)phdr[i].p_paddr;

            VPTR src = (VPTR)((USINT8*)elfData + phdr[i].p_offset);

            memcpy(dest, src, phdr[i].p_filesz);

            if(phdr[i].p_memsz > phdr[i].p_filesz) {
                memset((USINT8*)dest + phdr[i].p_filesz, 0, 
                        phdr[i].p_memsz - phdr[i].p_filesz);
            }
        }
    }
    return STATUS_OK;
}


