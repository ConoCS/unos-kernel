#pragma once
#include <stdint.h>
#include "../boot/bootinfo.h"

typedef unsigned long size_t;

#define KERNEL_HEAP_START 0xFFFF800000000000
#define KERNEL_HEAP_SIZE (10 * 1024 * 1024)
#define KERNEL_HEAP_PHYSICAL_START 0x00400000
#define PHYS_HEAP_START KERNEL_HEAP_PHYSICAL_START
#define PAGE_SIZE 0x1000

void map_identity(uint64_t physc_addr, uint64_t size);
void map_virtual(uint64_t virtual_addr, uint64_t phys_addr, uint64_t size);
void init_paging(BOOT_INFO *bootInfo);
void *kmalloc(size_t size);
void *palloc_aligned(size_t size, size_t align);
uint64_t virt_to_phys(uint64_t virtual_addr);
void *palloc_aligned_DMA(size_t size, size_t align, uintptr_t *phys_out);