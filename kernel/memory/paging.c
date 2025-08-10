#include <unoskrnl.h>
#include "drivers/storage/storage.h"

#define KERNEL_HEAP_START 0xFFFF800000000000 
#define KERNEL_HEAP_SIZE (10 * 1024 * 1024)
#define KERNEL_HEAP_PHYSC_START 0x00400000
#define PAGE_SUPERVISOR 0x0       // U/S = 0

static uint64_t *pml4_table = (uint64_t*)0x1000;
uint64_t *kernel_pml4 = NULL;

#define PAGE_SIZE 0x1000
#define PAGE_NOCACHE 0x10

// Pointer awal memori kosong untuk alokasi page table baru
static uint64_t *next_free_page = (uint64_t*)0x4000; // setelah 0x3000

// Alokasi page kosong 4KB, dan return pointer-nya
uint64_t* alloc_page() {
    uint64_t* page = next_free_page;
    next_free_page = (uint64_t*)((uint64_t)next_free_page + PAGE_SIZE);
    // Optional: clear isi page
    for (int i = 0; i < 512; i++) page[i] = 0;
    return page;
}

void map_identity(uint64_t physc_addr, uint64_t size) {
    for (uint64_t addr = physc_addr; addr < physc_addr + size; addr += 0x200000) {
        uint64_t pml_index   = (addr >> 39) & 0x1FF;
        uint64_t pdpt_index  = (addr >> 30) & 0x1FF;
        uint64_t pd_index    = (addr >> 21) & 0x1FF;

        // Jika belum ada PDPT, alokasikan
        if (!(pml4_table[pml_index] & PAGE_PRESENT)) {
            uint64_t* new_pdpt = alloc_page();
            pml4_table[pml_index] = (uint64_t)new_pdpt | PAGE_PRESENT | PAGE_WRITABLE | PAGE_SUPERVISOR;
        }

        uint64_t* pdpt = (uint64_t*)(pml4_table[pml_index] & 0x000FFFFFFFFFF000);

        // Jika belum ada PD, alokasikan
        if (!(pdpt[pdpt_index] & PAGE_PRESENT)) {
            uint64_t* new_pd = alloc_page();
            pdpt[pdpt_index] = (uint64_t)new_pd | PAGE_PRESENT | PAGE_WRITABLE | PAGE_SUPERVISOR;
        }

        uint64_t* pd = (uint64_t*)(pdpt[pdpt_index] & 0x000FFFFFFFFFF000);

        // Set mapping langsung (2MB page)
        pd[pd_index] = (addr & 0xFFFFFFFFFFE00000) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_LARGE | PAGE_SUPERVISOR;
    }
}

void map_framebuffer(uint64_t addr, uint64_t size) {
    for (uint64_t a = addr; a < addr + size; a += 0x200000) {
        uint64_t pml_index   = (a >> 39) & 0x1FF;
        uint64_t pdpt_index  = (a >> 30) & 0x1FF;
        uint64_t pd_index    = (a >> 21) & 0x1FF;

        if (!(pml4_table[pml_index] & PAGE_PRESENT)) {
            uint64_t* new_pdpt = alloc_page();
            pml4_table[pml_index] = (uint64_t)new_pdpt | PAGE_PRESENT | PAGE_WRITABLE;
        }

        uint64_t* pdpt = (uint64_t*)(pml4_table[pml_index] & 0x000FFFFFFFFFF000);

        if (!(pdpt[pdpt_index] & PAGE_PRESENT)) {
            uint64_t* new_pd = alloc_page();
            pdpt[pdpt_index] = (uint64_t)new_pd | PAGE_PRESENT | PAGE_WRITABLE;
        }

        uint64_t* pd = (uint64_t*)(pdpt[pdpt_index] & 0x000FFFFFFFFFF000);

        pd[pd_index] = (a & 0xFFFFFFFFFFE00000) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_LARGE | PAGE_PCD | PAGE_PWT;

        asm volatile ("invlpg (%0)" : : "r" (a) : "memory");
        
    }
}

void map_virtual(uint64_t virtual_addr, uint64_t phys_addr, uint64_t size) {
    //serial_printf("Start checkpoint MAP VIRTUAL\n");
    for(uint64_t offset = 0 ; offset < size ; offset += PAGE_SIZE) {
        uint64_t vaddr = virtual_addr + offset;
        uint64_t paddr = phys_addr + offset;

        uint64_t pml_index = (vaddr >> 39) & 0x1FF;
        uint64_t pdpt_index = (vaddr >> 30) & 0x1FF;
        uint64_t pd_index = (vaddr >> 21) & 0x1FF;
        uint64_t pt_index = (vaddr >> 12) & 0x1FF;
        

        // PML4
        if(!(pml4_table[pml_index] & PAGE_PRESENT)){
            uint64_t *new_pdpt = alloc_page();
            pml4_table[pml_index] = (uint64_t)new_pdpt | PAGE_PRESENT | PAGE_WRITABLE;
        }
        uint64_t *pdpt = (uint64_t*)(pml4_table[pml_index] & 0x000FFFFFFFFFF000);
        

        // PDPT
        if(!(pdpt[pdpt_index] & PAGE_PRESENT)) {
            uint64_t *new_pd = alloc_page();
            pdpt[pdpt_index] = (uint64_t)new_pd | PAGE_PRESENT | PAGE_WRITABLE;
        }
        uint64_t* pd = (uint64_t*)(pdpt[pdpt_index] & 0x000FFFFFFFFFF000);
        

        // PD
        if (!(pd[pd_index] & PAGE_PRESENT)) {
            uint64_t *new_pt = alloc_page();
            pd[pd_index] = (uint64_t)new_pt | PAGE_PRESENT | PAGE_WRITABLE;
        }
        uint64_t* pt = (uint64_t*)(pd[pd_index] & 0x000FFFFFFFFFF000);
        

        // PT (4KiB)
        pt[pt_index] = (paddr & 0x000FFFFFFFFFF000) | PAGE_PRESENT | PAGE_WRITABLE;
        
    }
}

void map_virtual_user(uint64_t virtual_addr, uint64_t phys_addr, uint64_t size) {
    for(uint64_t offset = 0 ; offset < size ; offset += PAGE_SIZE) {
        uint64_t vaddr = virtual_addr + offset;
        uint64_t paddr = phys_addr + offset;

        uint64_t pml_index = (vaddr >> 39) & 0x1FF;
        uint64_t pdpt_index = (vaddr >> 30) & 0x1FF;
        uint64_t pd_index = (vaddr >> 21) & 0x1FF;
        uint64_t pt_index = (vaddr >> 12) & 0x1FF;

        // PML4
        if(!(pml4_table[pml_index] & PAGE_PRESENT)){
            uint64_t *new_pdpt = alloc_page();
            memset(new_pdpt, 0, PAGE_SIZE);
            pml4_table[pml_index] = virt_to_phys_simple((uint64_t)new_pdpt) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
        }
        uint64_t *pdpt = (uint64_t*)(pml4_table[pml_index] & PAGE_MASK);

        // PDPT
        if(!(pdpt[pdpt_index] & PAGE_PRESENT)) {
            uint64_t *new_pd = alloc_page();
            memset(new_pd, 0, PAGE_SIZE);
            pdpt[pdpt_index] = virt_to_phys_simple((uint64_t)new_pd) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
        }
        uint64_t* pd = (uint64_t*)(pdpt[pdpt_index] & PAGE_MASK);

        // PD
        if (!(pd[pd_index] & PAGE_PRESENT)) {
            uint64_t *new_pt = alloc_page();
            memset(new_pt, 0, PAGE_SIZE);
            pd[pd_index] = virt_to_phys_simple((uint64_t)new_pt) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
        }
        uint64_t* pt = (uint64_t*)(pd[pd_index] & PAGE_MASK);

        // PT (4KiB) dengan flag USER untuk akses user mode
        pt[pt_index] = (paddr & PAGE_MASK) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
        asm volatile ("invlpg (%0)" : : "r" (vaddr) : "memory");

    }
}

VOID
map_virtual_user_advanced(USINT64 virtual_addr, USINT64 phys_addr, USINT64 size, USINT64 flags) {
    for(USINT64 offset = 0; offset < size; offset += PAGE_SIZE) {
        USINT64 vaddr = virtual_addr + offset;
        USINT64 paddr = phys_addr + offset;

        uint64_t pml_index = (vaddr >> 39) & 0x1FF;
        uint64_t pdpt_index = (vaddr >> 30) & 0x1FF;
        uint64_t pd_index = (vaddr >> 21) & 0x1FF;
        uint64_t pt_index = (vaddr >> 12) & 0x1FF;

        // PML4
        if(!(pml4_table[pml_index] & PAGE_PRESENT)) {
            USINT64 *new_pdpt = alloc_page();
            memset(new_pdpt, 0, PAGE_SIZE);
            pml4_table[pml_index] = virt_to_phys_simple((USINT64)new_pdpt) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
        }
        USINT64 *pdpt = (USINT64*)(pml4_table[pml_index] & PAGE_MASK);

        //PDPT
        if(!(pdpt[pdpt_index] & PAGE_PRESENT)) {
            USINT64 *new_pd = alloc_page();
            memset(new_pd, 0, PAGE_SIZE);
            pdpt[pdpt_index] = virt_to_phys_simple((USINT64)new_pd) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
        }
        USINT64 *pd = (USINT64*)(pdpt[pdpt_index] & PAGE_MASK);

        //PD
        if(!(pd[pd_index] & PAGE_PRESENT)) {
            USINT64 *new_pt = alloc_page();
            memset(new_pt, 0, PAGE_SIZE);
            pd[pd_index] = virt_to_phys_simple((USINT64)new_pt) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
        }
        USINT64 *pt = (USINT64*)(pd[pd_index] & PAGE_MASK);

        //PT 4KiB Page
        pt[pt_index] = (paddr & PAGE_MASK) | flags;
        Printk(KINFO, "Mapping VA 0x%x to PA 0x%x with flags 0x%x\n", vaddr, paddr, flags);

        // after setting: read back the PT entry and print
        uint64_t pte = pt[pt_index];
        Printk(KINFO, "PTE readback for VA 0x%x: 0x%x\n", (unsigned long long)vaddr, (unsigned long long)pte);
        // Invalidate the TLB entry for this virtual address
        asm volatile ("invlpg (%0)" : : "r" (vaddr) : "memory");
    }
}

uint64_t align_up_2mb(uint64_t size) {
    return (size + 0x1FFFFF) & ~0x1FFFFF;
}

static inline void* phys_to_virt(uint64_t phys_addr) {
    // Misal kernel mapping physical RAM mulai dari KERNEL_VIRT_BASE
    return (void*)(phys_addr + KERNEL_VIRT_BASE - KERNEL_PHYS_BASE);
}

void dump_pte_for_vaddr(uint64_t vaddr) {
    uint64_t pml = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt = (vaddr >> 30) & 0x1FF;
    uint64_t pd  = (vaddr >> 21) & 0x1FF;
    uint64_t pt  = (vaddr >> 12) & 0x1FF;

    uint64_t pml_e = pml4_table[pml];
    uint64_t *pdpt_table = (uint64_t*)(phys_to_virt(pml_e & PAGE_MASK));
    uint64_t pdpt_e = pdpt_table[pdpt];
    uint64_t *pd_table = (uint64_t*)(phys_to_virt(pdpt_e & PAGE_MASK));
    uint64_t pd_e = pd_table[pd];
    uint64_t *pt_table = (uint64_t*)(phys_to_virt(pd_e & PAGE_MASK));
    uint64_t pte = pt_table[pt];

    serial_printf("Dump vaddr %p:\nPML4e=0x%016llx\nPDPTe=0x%016llx\nPDe=0x%016llx\nPTe=0x%016llx\n",
                  vaddr, pml_e, pdpt_e, pd_e, pte);

    serial_printf("PTE flags: present=%d write=%d user=%d NX=%d\n",
        (pte & 1) ? 1:0,
        (pte & 2) ? 1:0,
        (pte & 4) ? 1:0,
        (pte >> 63) & 1);
}


void init_paging(BOOT_INFO *bootInfo) {
    // alok PML4
    pml4_table = alloc_page();
    memset(pml4_table, 0, PAGE_SIZE);
    serial_print("Start mapping PML4\n");
    // MAP Kernel Identity
    extern uint64_t _kernel_start;
    extern uint64_t _kernel_end;

    uint64_t kernel_phys_start = (uint64_t)&_kernel_start;
    uint64_t kernel_phys_end = (uint64_t)&_kernel_end;

    uint64_t kernel_phys_size = kernel_phys_end - kernel_phys_start;
    map_identity(kernel_phys_start, kernel_phys_size);
    map_identity(0x4000, 0x100000);
    
    // PHYS_Heap_START
    map_virtual(KERNEL_HEAP_START, KERNEL_HEAP_PHYSICAL_START, KERNEL_HEAP_SIZE);
    serial_print("Kernel identity mapping done");

    // Map framebuffer GOP
    map_framebuffer(bootInfo->GopBootInform->gop_framebase,
      bootInfo->GopBootInform->gop_framesize);
    serial_print("map_identity Map framebuffer GOP... \n");

    // Optional: map ACPI & BootInfo struct juga
    map_identity((uint64_t)bootInfo, 0x200000);
    map_identity((uint64_t)bootInfo->GopBootInform, 0x1000);
    map_identity((uint64_t)bootInfo->AcpiBootInform, sizeof(ACPI_BOOT_INFO));

    serial_print("map_identity map ACPI & BootInfo... \n");

    serial_print("\nBootInfo PTR: "); serial_print_hex((uint64_t)bootInfo); serial_print("\n");
    serial_print("Gop PTR: "); serial_print_hex((uint64_t)bootInfo->GopBootInform); serial_print("\n");
    serial_print("Acpi PTR: "); serial_print_hex((uint64_t)bootInfo->AcpiBootInform); serial_print("\n");
    
    // Mapping alamat AHCI PCI agar bisa dipake buat DMA
    map_identity(AHCI_BASE, 0x200000); // AHCI_BASE
    map_identity(abar_global, 0x200000);  // ABAR_BASE

    // — map region ACPI/MADT secara tepat —
    uint64_t madt_phys = (uint64_t)bootInfo->AcpiBootInform->ACPIMADT;
    ACPI_MADT *madt_hdr = (ACPI_MADT *)madt_phys;
    uint32_t madt_len  = madt_hdr->Header.Length;

    // page-align: start turun ke boundary 4 KiB, end naik ke boundary 4 KiB
    uint64_t map_start = madt_phys & ~0xFFFULL;
    uint64_t map_end   = ( (madt_phys + madt_len + 0xFFFULL) & ~0xFFFULL );
    uint64_t map_size  = map_end - map_start;

    map_identity(map_start, map_size);
    serial_printf("Mapped MADT %X (+%u bytes) → pages [0x%lX–0x%lX)\n",
                  map_start, madt_len, map_start, map_end);
    map_identity((uint64_t)IoAPICAddress, 0x1000);
    serial_printf("Mapped IOAPIC address at %x\n", IoAPICAddress);
    map_identity(0xFEC00000, 0x1000);
    map_identity(0xFEE00000, 0x1000);
    map_identity(0xFEC000, 0x1000);

    // activate mapping for userland
    //map_virtual_user(USERLAND_VIRTUAL_MAPPING, 0x0400000, 0x10000000);
    map_virtual_user(USERLAND_STACK - USERLAND_STACK_SIZE, 0x05000000, USERLAND_STACK_SIZE);
    map_virtual_user(USERLAND_HEAP_BASE, 0x06000000, USERLAND_HEAP_SIZE);
    serial_printf("Done mapping userland\n");

    // Activate paging
    serial_print("PML4 Mapping done\n");
    load_pml4(pml4_table);
    kernel_pml4 = pml4_table;
    serial_print("Paging activated\n");
    serial_printf("Paging active at: %X\n\n", pml4_table);
}

uint64_t virt_to_phys(uint64_t virtual_addr) {
    uint64_t pml_index = (virtual_addr >> 39) & 0x1FF;
    uint64_t pdpt_index = (virtual_addr >> 30) & 0x1FF;
    uint64_t pd_index = (virtual_addr >> 21) & 0x1FF;
    uint64_t offset = virtual_addr & 0x1FFFFF;

    uint64_t *pdpt = (uint64_t*)(pml4_table[pml_index] & 0x000FFFFFFFFFF000);
    if(!pdpt) return 0;

    uint64_t *pd = (uint64_t*)(pdpt[pdpt_index] & 0x000FFFFFFFFFF000);
    if (!pd) return 0;

    uint64_t pde = pd[pd_index];
    if(!(pde & PAGE_PRESENT)) return 0;

    uint64_t phys_base = pde & 0xFFFFFFFFFFE00000;

    return phys_base + offset;
}


uint64_t virt_to_phys_simple(uint64_t virt) {
    if (virt >= KERNEL_VIRT_BASE) {
        Printk(KSUCCESS, "Returning virt\n");
        return virt - KERNEL_VIRT_BASE + KERNEL_PHYS_BASE;
    } else {
        // Kalau bukan area kernel heap, return 0 atau handle lain
        Printk(KWARN, "Return 0 at virt to phys simple\n");
        return 0;
    }
}