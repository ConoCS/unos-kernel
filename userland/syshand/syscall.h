#ifndef _UNOS_SYSCALL_
#define _UNOS_SYSCALL_

#include <unostype.h>

enum {
    SYSCALL_WRITE = 0,
    SYSCALL_READ = 1,
    SYSCALL_OPEN = 2,
    SYSCALL_EXIT = 3,
    SYSCALL_COUNT
};

VOID SyscallHandler(USINT64 RAX, USINT64 RDI, USINT64 RSI, USINT64 RDX);

#endif