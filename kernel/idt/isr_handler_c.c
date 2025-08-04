#include <unoskrnl.h>

/* FUNGSI PANIC BISA DIPANGGIL SEBAGAI BERIKUT 
    RaiseKernelPanicError(error_msg, 14)    */

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

/* SEND EOI FUNCTION UNTUK MENGIRIMKAN END OF INTERRUPT*/
void send_eoi(uint8_t irq) {
    if (irq >= 8) 
        outb(0xA0, 0x20);
    outb(0x20, 0x20);
}


/* HANDLER UNTUK SEMUA ISR */

__attribute__((noreturn))
void isr0_handler_c() {
    serial_print("EXCEPTION: Divide by zero\n");
    while(1) {
        __asm__("hlt");
    }
}

__attribute__((noreturn))
void isr1_handler_c() {
    while(1) {
        __asm__("hlt");
    }
}

__attribute__((interrupt))
void isr13_handler_c(struct interrupt_frame *frame, uint64_t error_code) {
    RaiseKernelPanicError(error_code, CPU_FAULT_ACTIVATE_GENERAL_PROTECTION);
    serial_print("EXCEPTION: Exception happened\n");
    serial_print("PLEASE HARD RESET YOUR COMPUTER. THE KERNEL IS HALTING\n\n");
    serial_print("ERROR: CPU_FAULT_ACTIVATE_GENERAL_PROTECTION\n");
    while(1) {
        __asm__("hlt");
    }
}

static inline void short_pause() {
    for (volatile int i = 0; i < 10000000; i++) __asm__ __volatile__("pause");
}

__attribute__((noreturn))
void isr14_handler_c(uint64_t error_code) {
    RaiseKernelPanicError(error_code, MEMORY_PAGE_FAULT_PAGE_UNREADY);
    short_pause();
    serial_print("EXCEPTION: Exception happened \n");
    serial_print("PLEASE HARD RESET YOUR COMPUTER. THE KERNEL IS HALTING\n\n");
    serial_print("ERROR: MEMORY_PAGE_FAULT_PAGE_UNREADY\n");
    serial_printf("Error Code: %X\n", error_code);

    __asm__ __volatile__ (
        "cli\n\t"
        "mov $0xFE, %%al\n\t"
        "out %%al, $0x64\n\t"
        "hlt\n\t"
        "jmp .\n\t"
        :
        :
        : "al"
    );

    __builtin_unreachable(); 

}

volatile uint64_t tick = 0;

void irq_handler_c(registers_t *regs, uint64_t irq_number) {
    apic_send_eoi();

    // increment scaled tick
    for (int i = 0; i < SCALE_FACTOR; i++) {
        tick++;
        if (tick % 100 == 0) {
            second++;
            if (second == 60) {
                second = 0;
                minute++;
            }
        }
    }

    WatchdogCountPlus(WatchdogUnOSKrnl);
    DetectWatchdog(WatchdogUnOSKrnl);
}

uint64_t gettick_handler() {
    return tick;
}

void delay(int ms) {
    int target_tick = tick + (ms * 100 / 1000);  // karena 100 tick = 1000ms
    while (tick < target_tick);
}


