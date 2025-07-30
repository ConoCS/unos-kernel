#include <stdint.h>
#include "idt.h"
#include "errornumber/errnum.h"
#include "../Include/com.h"

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
    // RaiseKernelPanicError(error_code, CPU_FAULT_ACTIVATE_GENERAL_PROTECTION);
    serial_print("EXCEPTION: Exception happened\n");
    serial_print("PLEASE HARD RESET YOUR COMPUTER. THE KERNEL IS HALTING\n\n");
    serial_print("ERROR: CPU_FAULT_ACTIVATE_GENERAL_PROTECTION\n");
    while(1) {
        __asm__("hlt");
    }
}

__attribute__((noreturn))
void isr14_handler_c(uint64_t error_code) {
    // RaiseKernelPanicError(error_code, MEMORY_PAGE_FAULT_PAGE_UNREADY);
    serial_print("EXCEPTION: Exception happened \n");
    serial_print("PLEASE HARD RESET YOUR COMPUTER. THE KERNEL IS HALTING\n\n");
    serial_print("ERROR: MEMORY_PAGE_FAULT_PAGE_UNREADY\n");
    serial_printf("Error Code: %X\n", error_code);
    
    while(1) {
        __asm__("hlt");
    }
}

int tick = 0;

void irq_handler_c(registers_t *regs, uint64_t irq_number) {
    // Kirim EOI ke PIC
    if (irq_number >= 40) {
        outb(0xA0, 0x20); // Slave PIC
    }
    outb(0x20, 0x20); // Master PIC

    tick++;
    //serial_print("TICK: ");
    //serial_print_hex((uint64_t)tick);
    //serial_print("\n");

    //if (tick % 100 == 0) {
        //serial_print("1 Second passed...\n");
    //}
}

uint64_t gettick_handler() {
    return tick;
}


