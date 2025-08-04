#include <unoskrnl.h>

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

    // Save mask
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);

    // Start init
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    outb(PIC1_DATA, 0x20); // Master offset
    outb(PIC2_DATA, 0x28); // Slave offset

    outb(PIC1_DATA, 0x04); // Tell Master about Slave at IRQ2
    outb(PIC2_DATA, 0x02); // Tell Slave its cascade identity

    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // Restore mask, tapi unmask IRQ0 dan IRQ1
    a1 &= ~(1 << 0); // Unmask IRQ0
    a1 &= ~(1 << 1); // Unmask IRQ1
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);

    // Delay ~ few µs
    for (volatile int i = 0; i < 1000; i++);

    serial_print("PIC Remap has done. Status: OK\n");
}

void disable_pic() {
    // Mask semua IRQ
    outb(0x21, 0xFF); // Master PIC
    outb(0xA1, 0xFF); // Slave PIC
    serial_print("PIC disabled.\n");
}