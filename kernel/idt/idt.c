#include <unoskrnl.h>

struct IDTEntry{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct IDTDescriptor {
    uint16_t size;
    uint64_t base;
} __attribute__((packed));

extern void isr0();
extern void isr1();
extern void isr13();
extern void isr14();
extern void isr32();
extern void isr33();
extern void isr_mouse();
GLOBAL VOID syscall_handler_wrapper();
static struct IDTEntry idt[256];

volatile uint32_t minute = 0;
volatile uint8_t second = 0;

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


static void set_idt_entry(int vector, void(*handler)(), INT DPL) {
    uint64_t addr = (uint64_t)handler;

    idt[vector].offset_low = addr & 0xFFFF;
    idt[vector].selector = 0x08;
    idt[vector].ist = 0;

    idt[vector].type_attr = 0x8E | ((DPL & 0x03) << 5);

    idt[vector].offset_mid = (addr >> 16) & 0xFFFF;
    idt[vector].offset_high = (addr >> 32);
    idt[vector].zero = 0;
}

void init_idt() {
    set_idt_entry(0, isr0, 0);
    set_idt_entry(1, isr1, 0);
    set_idt_entry(13, isr13, 0);
    set_idt_entry(14, isr14, 0);
    set_idt_entry(0x20, isr32, 0);
    set_idt_entry(0x21, isr33, 0);
    set_idt_entry(0x2C, isr_mouse, 0);
    set_idt_entry(0x80, syscall_handler_wrapper, 3);

    struct IDTDescriptor idtr = {
        .size = sizeof(idt) - 1,
        .base = (uint64_t)&idt
    };

    __asm__ volatile("lidt %0" : : "m"(idtr));

    serial_print("Sucessfully activating IDT \n");
}