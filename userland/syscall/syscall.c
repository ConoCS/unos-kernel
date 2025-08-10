#include <unoskrnl.h>

USINT64
SyscallHandler(
    USINT64 syscall_number
)
{
    serial_printf("[INFO] Syscall happened\n");
    return 1;
}