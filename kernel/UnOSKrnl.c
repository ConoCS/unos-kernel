#include <unoskrnl.h>

#define COM1 0x3F8
/* START COM SERIAL PRINT OUTPUT */

SINLINE VOID outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

SINLINE USINT8 inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" 
            : "=a"(ret)
            : "Nd"(port));
    return ret;
}

VOID init_serial() {
    outb(COM1 + 1, 0x00); // disable interrupt
    outb(COM1 + 3, 0x80); // aktifkan DLAB
    outb(COM1 + 0, 0x03); // set divisor to 3 (lo byte) 38400 baud
    outb(COM1 + 1, 0x00); //                  (hi byte)
    outb(COM1 + 3, 0x03); // 8 Bits, no parity, one stop hit
    outb(COM1 + 2, 0xC7); // ENABLe FIFO, clear them with 14 bytes threshold
    outb(COM1 + 4, 0x0B); // IRQs Enabled
}

INT is_transit_empty() {
    return inb(COM1 + 5) & 0x20;
}

VOID serial_write_char (char a) {
    while(is_transit_empty() == 0); 
    outb(COM1, a);
    
}

VOID serial_print(const char *str) {
    while(*str) {
        if(*str == '\n') serial_write_char('\r');
        serial_write_char(*str++);
    }
}

VOID serial_print_hex(uint64_t value) {
    CHARA8 hex[] = "0123456789ABCDEF";
    CHARA8 buffer[17];
    buffer[16] = '\0'; 

    for (INT i = 15; i >= 0; i--) {
        buffer[i] = hex[value & 0xF];
        value >>= 4;
    }

    serial_print("0x");
    serial_print(buffer);
}

VOID serial_printf(const char *fmt, ...) {
    va_list args;
    va_start (args, fmt);

    while(*fmt) {
        if(*fmt == '%') {
            fmt++;
            //Parse flag
            BOOL zero_pad = FALSE;
            if (*fmt == '0') {
                zero_pad = TRUE;
                fmt++;
            }

            //parse width
            INT width = 0;
            while(*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }

            CHARA8 buf[32];
            const char *str = NULL;

            switch (*fmt) {
                case 's':
                    {str = va_arg(args, const char*);
                    serial_print(str);
                    break;}
                case 'c':
                    {serial_write_char((char)va_arg(args, int));
                    break;}
                case 'd':
                case 'i':
                    {int val = va_arg(args, int);
                    itoa(val, buf);
                    int len = strlen(buf);
                    for (int i = len; i < width; i++) serial_write_char(zero_pad ? '0' : ' ');
                    serial_print(buf);
                    break;}
                case 'u':
                    {unsigned int val = va_arg(args, unsigned int);
                    utoa(val, buf);
                    int len = strlen(buf);
                    for (int i = len; i < width; i++) serial_write_char(zero_pad ? '0' : ' ');
                    serial_print(buf);
                    break;}
                case 'x':
                    {int uppercase = (*fmt == 'X');
                    uint64_t val = va_arg(args, uint64_t);
                    xtoa(val, buf, uppercase);
                    int len = strlen(buf);
                    for (int i = len; i < width; i++) serial_write_char(zero_pad ? '0' : ' ');
                    serial_print(buf);
                    break;}
                case 'X':
                    {int uppercase = (*fmt == 'X');
                    uint64_t val = va_arg(args, uint64_t);
                    xtoa(val, buf, uppercase);
                    int len = strlen(buf);
                    for (int i = len; i < width; i++) serial_write_char(zero_pad ? '0' : ' ');
                    serial_print(buf);
                    break;}
                case 'p':
                    {VOID *ptr = va_arg(args, VPTR);
                    xtoa((uint64_t)(uintptr_t)ptr, buf, 0);
                    serial_print("0x");
                    serial_print(buf);
                    break;}
                case 'l':
                    if (*(fmt + 1) == 'l' && *(fmt + 2) == 'u') {
                        fmt += 2;
                        unsigned long long val = va_arg(args, unsigned long long);
                        utoa(val, buf);
                        serial_print(buf);
                    } else {
                        serial_write_char('%');
                        serial_write_char('l');
                        serial_write_char(*(fmt + 1));
                        fmt+= 2;
                        continue;
                    }
                break;
                case '%':
                    serial_write_char('%');
                    break;
                default:
                    serial_write_char('%');
                    serial_write_char(*fmt);
                    
            }
        } else {
            serial_write_char(*fmt);
        }
        fmt++;
    }
    va_end(args);
}


/* END COM CODE*/

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
    for(int i = 0; i < sector_count * 512; i++) {
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
