#include <stdint.h>
#include "../Include/com.h"
#include "pic.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1
#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01
#define ICW4_8086    0x01

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" 
            : "=a"(ret)
            : "Nd"(port));
    return ret;
}

void remap_pic() {
    serial_print("Begin PIC REMAP\n\n");
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
    serial_print("Successfully saving last mask\n");

    // Mulai inisialisasi
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    serial_print("Start initializing PIC...\n");

    // Setting vector offset
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    serial_print("Setting vector offset...\n");

    // Setup chaining
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    serial_print("Setup chaining...\n");

    // Mode 8086
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
    serial_print("Setup 8086 mode...\n");

    // Pulihkan Mask
    outb(PIC1_DATA, 0xFE);
    outb(PIC2_DATA, 0xFF);
    serial_print("Restoring last mask that saved...\n");
    
    // Unmask IRQ0 (Timer)
    outb(PIC1_DATA, inb(PIC1_DATA) & ~0x01);  // Bit 0 = IRQ0
    serial_print("UnMasking IRQ0...\n");

    // Unmask IRQ1 (Keyboard)
    outb(PIC1_DATA, inb(PIC1_DATA) & ~(1 << 1)); 
    serial_print("UnMasking IRQ1...\n");


    serial_print("PIC Remap has done. Status: OK\n");
}