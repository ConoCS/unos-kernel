#define WRMSR_MACRO
#include <unoskrnl.h>

SINLINE
ASMWRMSR
VOID
WRMSR(
    IN USINT32 MSR,
    IN USINT64 VALUE
)
{
    USINT32 LOW = (USINT32)(VALUE & 0xFFFFFFFF);
    USINT32 HI = (USINT32)(VALUE >> 32);

    asm volatile(
        "wrmsr"
        :
        : "c"(MSR), "a"(LOW), "d"(HI)
    );
}

UNFUNCTION
SetupSyscallWrmsr(
    IN USINT64 SyscallHandlerAddr
)
{
    // Setup IA32_STAR: format
    // [63:48] Kernel CS (shift 48)
    // [47:32] User CS  (shift 32)
    // Biasanya kernel CS = 0x08, user CS = 0x1B (ring3)
    USINT64 Star = ((USINT64)0x08 << 32) | ((USINT64)0x1B << 48);
    WRMSR(IA32_STAR, Star);

    WRMSR(IA32_LSTAR, SyscallHandlerAddr);

    WRMSR(IA32_FMASK, 1 << 9);

    Printk(KSUCCESS, "[WRMSR] Success\n");
}

