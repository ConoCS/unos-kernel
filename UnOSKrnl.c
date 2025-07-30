#include "idt/idt.h"
#include "boot/bootinfo.h"
#include "Include/paging.h"
#include "Include/graphic.h"
#include "Include/scheduler.h"
#include "shceduler/tasklist.h"
#include "pic/pic.h"
#include "timer/pit.h"
#include "terminal/terminal.h"
#include "Include/string.h"
#include "driver/pci/pci.h"
#include "driver/storage.h"
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#define COM1 0x3F8

/* START COM SERIAL PRINT OUTPUT */

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" 
            : "=a"(ret)
            : "Nd"(port));
    return ret;
}

void init_serial() {
    outb(COM1 + 1, 0x00); // disable interrupt
    outb(COM1 + 3, 0x80); // aktifkan DLAB
    outb(COM1 + 0, 0x03); // set divisor to 3 (lo byte) 38400 baud
    outb(COM1 + 1, 0x00); //                  (hi byte)
    outb(COM1 + 3, 0x03); // 8 Bits, no parity, one stop hit
    outb(COM1 + 2, 0xC7); // ENABLe FIFO, clear them with 14 bytes threshold
    outb(COM1 + 4, 0x0B); // IRQs Enabled
}

int is_transit_empty() {
    return inb(COM1 + 5) & 0x20;
}

void serial_write_char (char a) {
    while(is_transit_empty() == 0); 
    outb(COM1, a);
    
}

void serial_print(const char *str) {
    while(*str) {
        if(*str == '\n') serial_write_char('\r');
        serial_write_char(*str++);
    }
}

void serial_print_hex(uint64_t value) {
    char hex[] = "0123456789ABCDEF";
    char buffer[17];
    buffer[16] = '\0'; 

    for (int i = 15; i >= 0; i--) {
        buffer[i] = hex[value & 0xF];
        value >>= 4;
    }

    serial_print("0x");
    serial_print(buffer);
}

void serial_printf(const char *fmt, ...) {
    va_list args;
    va_start (args, fmt);

    while(*fmt) {
        if(*fmt == '%') {
            fmt++;
            char buf[32];
            switch (*fmt) {
                case 's':
                    serial_print(va_arg(args, const char*));
                    break;
                case 'c':
                    serial_write_char((char)va_arg(args, int));
                    break;
                case 'd':
                case 'i':
                    itoa(va_arg(args, int), buf);
                    serial_print(buf);
                    break;
                case 'u':
                    utoa(va_arg(args, unsigned int), buf);
                    serial_print(buf);
                    break;
                case 'x':
                    serial_print("0x");
                    xtoa(va_arg(args, uint64_t), buf, 0);
                    serial_print(buf);
                    break;
                case 'X':
                    serial_print("0x");
                    xtoa(va_arg(args, uint64_t), buf, 1);
                    serial_print(buf);
                    break;
                case 'p':
                    void *ptr = va_arg(args, void*);
                    serial_print("0x");
                    xtoa((uint64_t)(uintptr_t)ptr, buf, 0);
                    serial_print(buf);
                    break;
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
                        fmt++;
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


void KernelPreSetup() {
    remap_pic();
    init_idt(); /*SETUP IDT*/
    init_pit(100);
    pci_scan();
}

__attribute__((noreturn))
void KernelMain(BOOT_INFO *BootInfoArg) {
    init_serial();
    serial_print("Copyright UNT University 1998 - 2025\n");
    serial_print("UnOS Krnl since 2025!\n\n");
    serial_print("Kernel started\n");

    KernelPreSetup();
     __asm__ volatile("sti");

    init_paging(BootInfoArg);
    gopInfo = BootInfoArg->GopBootInform;
    acpiInfo = BootInfoArg->AcpiBootInform;

    connect_to_framebuffer();

    serial_print("MemoryMapSize: ");
    serial_print_hex(BootInfoArg->MemoryMapSize);
    serial_print("\n");

    serial_print("Memory RAM: ");
    serial_print_hex(BootInfoArg->TotalAllRam);
    serial_print("\n");

    init_terminal();

    while(1) {
        asm("hlt");
    }
}
