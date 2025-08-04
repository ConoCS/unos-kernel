#include <unoskrnl.h>


#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void init_pit(uint32_t frequency) {
    serial_print("PIT Initializing...\n\n");
    serial_print("Frequency requested: "); serial_print_hex((uint64_t)frequency); serial_print("\n");
    uint16_t divisor = PIT_FREQUENCY / frequency;

    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
    serial_print("PIT Initializing done. Status: OK\n");
}
