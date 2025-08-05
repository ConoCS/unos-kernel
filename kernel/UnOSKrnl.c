/**
 *  Hak Cipta Dilindungi (c) 2025 UnOS Team of ConoCS
 *
 *  Lisensi: GPL v3.0
 *  Semua kode dalam file ini dan modul kernel terkait harus bersifat
 *  FREE dan OPEN SOURCE, sesuai dengan lisensi tersebut.
 *
 *  Nama File:
 *      UnOSKrnl.c
 *
 *  Ringkasan:
 *      Kernel MAIN
 *
 *  Penulis:
 *      Bernand, ??
 *
 *  Penafian:
 *      Penulis bertanggung jawab apabila modul ini menyebabkan
 *      kerusakan, kehilangan, atau ketidakstabilan pada kernel.
 *
 *  Histori Revisi:
 *      Untracked Revision (Maybe) : 10 - 30 times.
 *      
 */

#include <unoskrnl.h>

/* GLOBALLLLL */
 GOP_BOOT_INFO *gopInfo;
 ACPI_BOOT_INFO *acpiInfo;
 BOOT_INFO *bootInfo;


VOID TryAhciRead(IN HBA_PORT *port, IN uint64_t start_lba, IN uint32_t sector_count) {
    UINTPTR phys;
    VOID *virt = palloc_aligned_DMA(sector_count * 512, PAGE_SIZE, &phys);

    INT32 ok = ahci_read(port, start_lba, sector_count, (VPTR*)phys);
    if(ok == 1) serial_print("AHCI_READ OK\n");

    /* PRINT DATA */
    USINT8 *data = (USINT8*)virt;
    for(USINT32 i = 0; i < sector_count * 512; i++) {
        serial_printf("%X ", data[i]);
        if (i % 16 == 0 && i > 0) {
            serial_printf("\n");
        }
    }
    serial_printf("\n");
}

VOID KernelPreSetup(IN BOOT_INFO *Info) {
    remap_pic();
    init_idt(); /*SETUP IDT*/
    pci_scan();
    init_paging(Info); 
    bootInfo = Info;
    gopInfo = Info->GopBootInform;
    acpiInfo = Info->AcpiBootInform;
    init_phys_allocator();
    init_acpi();

    disable_pic();
    InitIOAPIC();
    USINT32 bus_hz = CalibrateLAPICBusHz();
    SetupLapicTimer(bus_hz);
    serial_printf("LAPIC Bus Hz: %u\n", bus_hz);
    LAPICEnable();
    InitWatchdog(0, 200, "UnOSKrnl", WDCallback);
} 

__attribute__((noreturn))
VOID KernelMain(BOOT_INFO *BootInfoArg) {
    __asm__ volatile("cli");
    init_serial();
    serial_print(ANSI_BOLD_ON "UnOS Kernel (C) 2025 ConoCS | GPLv3 Licensed\n\n" ANSI_BOLD_OFF);
    serial_print(ANSI_BOLD_ON"Kernel started\n"ANSI_BOLD_OFF);

    KernelPreSetup(BootInfoArg);

    __asm__ volatile("sti");
    connect_to_framebuffer();
    drawfullscreen(0x000000FF);
    ShowBGRT(BootInfoArg->AcpiBootInform->ACPIBGRT);

    serial_printf("Checkpoint 1\n");
    serial_printf("Pointer to AHCI PORT: %p", ahci_port);
    ParseGPT(ahci_port);
    
    VFSNode *root = kmalloc(sizeof(VFSNode));
    VFSMountFAT32Root(ahci_port, root);
    vfs_root = root;

    InitPSFFontGraphic();
    PreparingGlobalPSFFont();
    DrawSimplePSFText("UnOS Kernel (C) 2025 ConoCS", 10, 10, 0xFFFFFFFF);
    DrawSimplePSFText("GPLv3 Licensed", 10, 30, 0xFFFFFFFF);
    DrawSimplePSFText("Kernel started", 10, 50, 0xFFFFFFFF);
    
    DisableWatchdogWhile(WatchdogUnOSKrnl);

    init_terminal();

    while(1) {
        asm("hlt");
    }
}
