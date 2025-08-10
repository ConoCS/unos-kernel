#include <unoskrnl.h>

#define MAX_TRACKED_PAGES 8192 // misalnya 32MB / 4KB
#define DMA_VIRT_SIZE (512 * 1024 * 1024ULL) //512MB;
static const uint64_t dma_virt_base = KERNEL_DMA_VIRT_BASE;
static const uint64_t dma_virt_end  = dma_virt_base + DMA_VIRT_SIZE;
static uint64_t dma_heap_curr = dma_virt_base;

typedef unsigned long size_t;

uint64_t heap_curr = KERNEL_HEAP_START;
uint8_t page_bitmap[MAX_TRACKED_PAGES];
uint8_t dma_bitmap[DMA_MAX_PAGES] = {0};
uintptr_t base_phys_addr = 0x100000;
extern uint64_t *kernel_pml4;


int last_allocated_index = 0;

void *palloc_page() {
    for (int i = last_allocated_index; i < MAX_TRACKED_PAGES; i++) {
        if (!page_bitmap[i]) {
            page_bitmap[i] = 1;
            last_allocated_index = i + 1;
            return (void*)(base_phys_addr + i * PAGE_SIZE);
        }
    }

    // Backtrack untuk reuse yang sudah di-free
    for (int i = 0; i < last_allocated_index; i++) {
        if (!page_bitmap[i]) {
            page_bitmap[i] = 1;
            last_allocated_index = i + 1;
            return (void*)(base_phys_addr + i * PAGE_SIZE);
        }
    }

    Printk(KERR, "[palloc_page] Out of physical pages!\n");
    return NULL;
}

void *dma_alloc_page() {
    for (int i = 0; i < DMA_MAX_PAGES; i++) {
        if (!dma_bitmap[i]) {
            dma_bitmap[i] = 1;
            return (void*)(DMA_BASE_ADDR + i * PAGE_SIZE);
        }
    }
    return NULL;
}


void free_phys_page(void *addr) {
    uintptr_t phys = (uintptr_t)addr;
    if(phys < base_phys_addr) return;

    int index = (phys - base_phys_addr) / PAGE_SIZE;
    if(index >= 0 && index < MAX_TRACKED_PAGES) {
        page_bitmap[index] = 0;
    }
}

size_t align_up(size_t size, size_t align) {
    if (align < 1) return size;
    return (size + align - 1) & ~(align - 1);
}

void *kmalloc(size_t size) {
    // 1. align heap_curr
    uint64_t heap_curr_aligned = align_up(heap_curr, HEAP_ALIGN);
    // 2. align size (opsional, tapi safer)
    size = align_up(size, HEAP_ALIGN);

    // 3. pakai alamat aligned sebagai ptr
    void *ptr = (void*)heap_curr_aligned;
    // 4. update heap_curr ke akhir block
    heap_curr = heap_curr_aligned + size;
    return ptr;
}

static uint64_t next_free_phys = PHYS_HEAP_START;

static inline void invlpg(void *addr) {
    asm volatile("invlpg (%0)" : : "r" (addr) : "memory");
}

void pmm_free(void *phys_addr) {
    uintptr_t addr = (uintptr_t)phys_addr;
    if (addr < base_phys_addr) return;

    size_t index = (addr - base_phys_addr) / PAGE_SIZE;
    page_bitmap[index] = 0;
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

    uint64_t *pt = (uint64_t*)(pd[pd_index] & PAGE_MASK);
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

void *palloc_aligned_DMA(IN size_t size, size_t align, OUT uintptr_t *phys_out) {
    UNUSED(align);
    size_t map_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    if (dma_heap_curr & (PAGE_SIZE - 1)) {
        dma_heap_curr= (dma_heap_curr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    }

    if(dma_heap_curr + map_size > dma_virt_end){
        dma_heap_curr = dma_virt_base;
    }

    void *virt = (void*)dma_heap_curr;
    dma_heap_curr += map_size;

    uintptr_t phys = 0;
    for(size_t offset = 0; offset < map_size; offset += PAGE_SIZE) {
        void *page = dma_alloc_page();
        if (!page) return NULL;
        if (offset == 0) phys = (uintptr_t)page;
        map_virtual((uint64_t)virt + offset, (uintptr_t)page, PAGE_SIZE);
    }

    if (phys_out) *phys_out = phys;
   Printk(KINFO, "DMAALLOC: virt=%p, phys=%p, size=%u\n", virt, (void*)phys, map_size);
    return virt;
}


void pfree_aligned_DMA(void *virt, size_t size) {
    if(!virt || size == 0) return;
    size_t map_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    uint64_t va = (uint64_t)virt;

    for(size_t offset = 0; offset < map_size; offset += PAGE_SIZE) {
        uint64_t vaddr = va + offset;

        uintptr_t phys = get_physical_address((void*)vaddr);

        unmap_page((void*)vaddr);

        int dma_index = (phys - DMA_BASE_ADDR) / PAGE_SIZE;
        if (dma_index >= 0 && dma_index < DMA_MAX_PAGES) {
            dma_bitmap[dma_index] = 0; 
        }
    }
}

void init_phys_allocator() {
    // Hitung berapa page kernel yang perlu diresevasi
    extern uint64_t _kernel_start;
    extern uint64_t _kernel_end;

    uintptr_t kernel_start = (uintptr_t)&_kernel_start;
    uintptr_t kernel_end = (uintptr_t)&_kernel_end;

    // Tandai semua halaman kernel
    for (uintptr_t addr = kernel_start; addr < kernel_end; addr += PAGE_SIZE) {
        size_t index = (addr - base_phys_addr) / PAGE_SIZE;
        if (index < MAX_TRACKED_PAGES)
            page_bitmap[index] = 1;
    }

    // (Opsional) mark AHCI/ABAR area kalau kamu identitas-mapping juga
    for (uintptr_t addr = AHCI_BASE; addr < AHCI_BASE + 0x200000; addr += PAGE_SIZE) {
        size_t index = (addr - base_phys_addr) / PAGE_SIZE;
        if (index < MAX_TRACKED_PAGES)
            page_bitmap[index] = 1;
    }

    for (uintptr_t addr = abar_global; addr < abar_global + 0x200000; addr += PAGE_SIZE) {
        size_t index = (addr - base_phys_addr) / PAGE_SIZE;
        if (index < MAX_TRACKED_PAGES)
            page_bitmap[index] = 1;
    }

    for (uintptr_t addr = 0x400000; addr < 0x1000000; addr += PAGE_SIZE) {
    size_t index = (addr - base_phys_addr) / PAGE_SIZE;
    if (index < MAX_TRACKED_PAGES)
        page_bitmap[index] = 1;
}
}
