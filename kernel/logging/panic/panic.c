#include <unoskrnl.h>

/**
 * @brief Panic
 * The PANIC function is used to raise a critical error and display a red screen,
 * indicating that the kernel has encountered a serious problem. This is very important.
 *
 */

UNFUNCTION
Panic(IN KernelPanic ErrorLevel) {
    asm volatile("cli");

    Printk(KERR, "Exception Happened\n");
    drawfullscreen(0xFFFF0000);
    ResetCursorX();
    ResetCursorY();

    SetCursorMiddle();
    PrintCentered("EXCEPTION: EXCEPTION HAPPENED\n");
    PrintCentered("PLEASE HARD RESET YOUR COMPUTER. THE KERNEL IS HALTING\n");

    switch(ErrorLevel){
        case WATCHDOG_TIMEOUT_TIMER_EXCEPTION: PrintCentered("WATCHDOG_TIMEOUT_TIMER_EXCEPTION\n"); break;
        case MEMORY_PAGE_FAULT_PAGE_UNREADY: PrintCentered("MEMORY_PAGE_FAULT_PAGE_UNREADY\n"); break;
        case CPU_FAULT_ACTIVATE_GENERAL_PROTECTION: PrintCentered("CPU_FAULT_ACTIVATE_GENERAL_PROTECTION\n"); break;
        case FAILED_INITIALIZATION_USERLAND: PrintCentered("FAILED_INITIALIZATION_USERLAND\n"); break;
        default: PrintCentered("\nKERNEL_PANIC\n");
    }

    for(int i = 0; i < 1000000; i++) {
        asm volatile("pause");
    }

    /** /Ignore/
     * Dan aku, kubayangkan dirimu
        Mulai ada rindu
        Duniaku terhenti kar'na kamu
        Mungkin bisa jadi milikku
        S'moga lagu cinta ini
        Bersarang tepat di hatimu
     */

    ResetLastCursorTTY();

    for(int i = 0; i < 1000000; i++) {
        asm volatile("pause");
    }
    

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

