#include <unoskrnl.h>

UNFUNCTION
UnStartUserland(
    IN VPTR EntryPoint,
    IN VPTR UserStack
)
{
    CONST USINT64 USER_CS = 0x1B;
    CONST USINT64 USER_DS = 0x23;

    asm volatile(
        "cli\n\t"
        // Set data segments ke user data selector
        "mov %0, %%ds\n\t"
        "mov %0, %%es\n\t"
        "mov %0, %%fs\n\t"
        "mov %0, %%gs\n\t"
        // Susun stack untuk iretq
        "pushq %0\n\t"     // SS (user data segment)
        "pushq %1\n\t"     // RSP (user stack)
        "pushfq\n\t"       // RFLAGS
        "pushq %2\n\t"     // CS (user code segment)
        "pushq %3\n\t"     // RIP (entry point)
        "iretq\n\t"
        :
        : "r"(USER_DS), "r"(UserStack), "r"(USER_CS), "r"(EntryPoint)
        : "memory"
    );
}