#include <stdint.h>
#include "../Include/paging.h"
#include "../Include/com.h"

#define PHYS_HEAP_START 0x1000000

typedef unsigned long size_t;

uint64_t heap_curr = KERNEL_HEAP_START;

void *kmalloc(size_t size) {
    void *ptr = (void*)heap_curr;
    heap_curr += size;
    return ptr;
}

static uint64_t next_free_phys = PHYS_HEAP_START;

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