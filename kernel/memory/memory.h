#pragma once
#include "kernel/bootinfo.h"
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
#define PAGE_PCD      (1ULL << 4)
#define PAGE_PWT      (1ULL << 3)
#define PAGE_NX       (1ULL << 63) // kalau support NX
#define HEAP_ALIGN 8 // atau 16 kalau mau aman untuk 64-bit
#define DMA_BASE_ADDR 0x8000000   // 16MB
#define DMA_MAX_PAGES 512        // misalnya 2MB DMA pool (512 x 4096)
#define KERNEL_DMA_VIRT_BASE  0xFFFF900000000000
#define CODE_FLAGS (PAGE_PRESENT | PAGE_USER)
#define DATA_FLAGS (PAGE_PRESENT | PAGE_USER | PAGE_WRITABLE)

// Userland
#define USERLAND_VIRTUAL_MAPPING 0x400000        // 4MB start for code
#define USERLAND_HEAP_BASE 0x2000000             // 32MB start for heap (jarak jauh dari 4MB)
#define USERLAND_STACK 0x7FFFFFF000               // tetap di 2GB-4KB
#define USERLAND_STACK_SIZE 0x8000                // 32KB stack
#define USERLAND_HEAP_SIZE (4 * 1024 * 1024)      // 4MB heap
#define USERLAND_STACK_BOTTOM USERLAND_STACK
#define USERLAND_STACK_TOP (USERLAND_STACK + USERLAND_STACK_SIZE)

// another define
#define KERNEL_VIRT_BASE KERNEL_HEAP_START
#define KERNEL_PHYS_BASE KERNEL_HEAP_PHYSICAL_START


GLOBAL uint64_t *kernel_pml4;
GLOBAL void load_pml4(uint64_t *pml4);

uint64_t* alloc_page();
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
uint64_t virt_to_phys_simple(uint64_t virt);
uint64_t phys_to_virt_simple(uint64_t phys);
void dump_pte_for_vaddr(uint64_t vaddr);
void map_virtual_user(uint64_t virtual_addr, uint64_t phys_addr, uint64_t size);
VOID
map_virtual_user_advanced(USINT64 virtual_addr, USINT64 phys_addr, USINT64 size, USINT64 flags);