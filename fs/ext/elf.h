#pragma once
#include <unostype.h>

#define EI_NIDENT 16
#define PT_LOAD 1
#define ELF_OK 0
#define ELF_ERR_BADMAGIC -1
#define ELF_ERR_UNSUPPORTED -2
#define ELF_ERR_NOMEM -3
#define ELF_ERR_MAPFAIL -4

#ifdef UNOS_ELF_USERLAND_MACRO

    #define ELFLOAD
    #define ELFSTART
    #define UNSTARTUSERLAND
    #define UNSYSCALLINIT

#endif

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;