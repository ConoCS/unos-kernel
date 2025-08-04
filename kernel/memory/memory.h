#pragma once
#include <unostype.h>

#define KERNEL_HEAP_START 0xFFFF800000000000
#define KERNEL_HEAP_SIZE (10 * 1024 * 1024)
#define KERNEL_HEAP_PHYSICAL_START 0x00400000
#define PHYS_HEAP_START KERNEL_HEAP_PHYSICAL_START
#define PAGE_SIZE 0x1000
#define PAGE_SHIFT 12
#define PAGE_MASK  0x000FFFFFFFFFF000ULL
#define PAGE_USER   0x004
// Bit flag untuk entri page table (x86_64 Paging)
#define PAGE_PRESENT 0x1 // Bit 0: Page aktif (1 = ada di RAM, 0 = akan trigger page fault)
#define PAGE_WRITABLE 0x2  // Bit 1: 1 = bisa ditulis, 0 = hanya bisa dibaca (read-only)
#define PAGE_LARGE 0x80 // Bit 7: 1 = page ini adalah huge page (2MB atau 1GB, tergantung level-nya)
#define HEAP_ALIGN 8 // atau 16 kalau mau aman untuk 64-bit
#define DMA_BASE_ADDR 0x8000000   // 16MB
#define DMA_MAX_PAGES 512        // misalnya 2MB DMA pool (512 x 4096)
#define KERNEL_DMA_VIRT_BASE  0xFFFF900000000000

void map_identity(uint64_t physc_addr, uint64_t size);
void map_virtual(uint64_t virtual_addr, uint64_t phys_addr, uint64_t size);
void init_paging(BOOT_INFO *bootInfo);
void *kmalloc(size_t size);
void *palloc_aligned(size_t size, size_t align);
uint64_t virt_to_phys(uint64_t virtual_addr);
void *palloc_aligned_DMA(size_t size, size_t align, uintptr_t *phys_out);
void pfree_aligned_DMA(void *virt, size_t size);
void init_phys_allocator();
void *palloc_page();
VOID map_user_page(USINT64 *pml4, USINT64 vaddr, USINT64 paddr, USINT64 flags);