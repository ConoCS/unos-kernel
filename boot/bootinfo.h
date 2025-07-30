#pragma once

#include <stdint.h>

typedef struct {
    void *ACPIBGRT;
    void *ACPIMADT;
    void *ACPIFADT;
    void* ACPIHPET;
}   ACPI_BOOT_INFO;

typedef struct {
    void *gop;
    uint64_t gop_framebase;
    uint32_t gop_framesize;
    uint32_t gop_width;
    uint32_t gop_pitch;
    uint32_t gop_height;
}   GOP_BOOT_INFO;

typedef struct {
    GOP_BOOT_INFO *GopBootInform;
    ACPI_BOOT_INFO *AcpiBootInform;
    void *MemoryMap;
    uint64_t MemoryMapSize;
    uint64_t MemoryMapDescriptorSize;
    uint64_t TotalAllRam;
}   BOOT_INFO;