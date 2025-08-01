#include <stdint.h>
#include "../Include/paging.h"
#include "../Include/com.h"
#include "../Include/string.h"

#define PHYS_HEAP_START 0x1000000

typedef unsigned long size_t;

uint64_t heap_curr = KERNEL_HEAP_START;
uint8_t page_bitmap[PAGE_SIZE];
uintptr_t base_phys_addr = 0x100000;

void *palloc_page() {
    for (int i = 0; i < PAGE_SIZE; i++) {
        if(page_bitmap[i] == 0) {
            page_bitmap[i] = 1;
            return (void*)(base_phys_addr + i * PAGE_SIZE);
        }
    }
    return NULL;
}

void free_phys_page(void *addr) {
    uintptr_t phys = (uintptr_t)addr;
    if(phys < base_phys_addr) return;

    int index = (phys - base_phys_addr) / PAGE_SIZE;
    if(index >= 0 && index < PAGE_SIZE) {
        page_bitmap[index] = 0;
    }
}

void *kmalloc(size_t size) {
    void *ptr = (void*)heap_curr;
    heap_curr += size;
    return ptr;
}

static uint64_t next_free_phys = PHYS_HEAP_START;

static inline void invlpg(void *addr) {
    asm volatile("invlpg (%0)" : : "r" (addr) : "memory");
}

void unmap_page(void *virtual_address) {
    uint64_t va = (uint64_t)virtual_address;

    size_t pml4_idx = (va >> 39) & 0x1FF;
    size_t pdpt_idx = (va >> 30) & 0x1FF;
    size_t pd_idx   = (va >> 21) & 0x1FF;
    size_t pt_idx   = (va >> 12) & 0x1FF;

    uint64_t *pdpt = (uint64_t*)(kernel_pml4[pml4_idx] & ~0xFFFULL);
    if(!(kernel_pml4[pml4_idx] & PAGE_PRESENT)) return;

    uint64_t *pd = (uint64_t*)(pdpt[pdpt_idx] & ~0xFFFULL);
    if(!(pdpt[pdpt_idx] & PAGE_PRESENT)) return;

    uint64_t *pt = (uint64_t*)(pd[pd_idx] & ~0xFFFULL);
    if(!(pd[pd_idx] & PAGE_PRESENT)) return;

    if(!(pt[pt_idx] & PAGE_PRESENT)) return;

    pt[pt_idx] = 0;

    invlpg((void*)va);
}

uint64_t get_physical_address(void *virt) {
    uint64_t virt_addr = (uint64_t)virt;

    uint16_t pml4_index = (virt_addr >> 39) & 0x1FF;
    uint64_t pdpt_index = (virt_addr >> 30) & 0x1FF;
    uint64_t pd_index = (virt_addr >> 21) & 0x1FF;
    uint64_t pt_index = (virt_addr >> 12) & 0x1FF;
    uint64_t offset = virt_addr & 0xFFF;

    uint64_t *pdpt = (uint64_t*)(kernel_pml4[pml4_index] & PAGE_MASK);
    if (!pdpt) return 0;

    uint64_t *pd = (uint64_t*)(pdpt[pdpt_index] & PAGE_MASK);
    if(!pd) return 0;

    uint64_t *pt = (uint64_t*)(pd[pd_index & PAGE_MASK]);
    if(!pt) return 0;

    uint64_t phys_page = pt[pt_index] & PAGE_MASK;
    if(!(pt[pt_index] & 1)) return 0;

    return phys_page + offset;
}

void *palloc_aligned(size_t size, size_t align) {
    if (align < 8) align = 8;

    // Align the current pointer
    if(next_free_phys % align != 0) {
        next_free_phys = (next_free_phys + align - 1) & ~(align - 1);
    } 

    void *addr = (void*)next_free_phys;
    next_free_phys += size;

    return addr;
}

void *palloc_aligned_DMA(size_t size, size_t align, uintptr_t *phys_out) {
    uintptr_t phys = (uintptr_t)palloc_aligned(size, align);

    size_t map_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    if(heap_curr & (PAGE_SIZE - 1)) {
        heap_curr = (heap_curr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    }

    void *virt = (void*)heap_curr;
    heap_curr += map_size;

    map_virtual((uint64_t)virt, phys, map_size);

    *phys_out = phys;
    return virt;
}

void pfree_aligned_DMA(void *virt, size_t size) {
    if(!virt || size == 0) return;
    size_t map_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    uint64_t va = (uint64_t)virt;

    for(size_t offset = 0; offset < map_size; offset += PAGE_SIZE) {
        uint64_t vaddr = va + offset;

        uintptr_t phys = get_physical_address(vaddr);

        unmap_page(vaddr);
    }
}