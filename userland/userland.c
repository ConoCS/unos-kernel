#include <unoskrnl.h>

VOID enter_userland(USINT64 rip, USINT64 rsp) {
    asm volatile (
        "cli \n\t"
        "mov $0x23, %%ax \n\t"      // 0x23 = user data segment selector (Ring 3)
        "mov %%ax, %%ds \n\t"
        "mov %%ax, %%es \n\t"
        "mov %%ax, %%fs \n\t"
        "mov %%ax, %%gs \n\t"

        "pushq $0x23 \n\t"          // SS (user data segment)
        "pushq %0 \n\t"             // RSP (user stack pointer)
        "pushfq \n\t"               // RFLAGS
        "pushq $0x1B \n\t"          // CS (user code segment)
        "pushq %1 \n\t"             // RIP (entry point user ELF)
        "iretq \n\t"
        :
        : "r"(rsp), "r"(rip)
        : "rax"
    );
}
