#ifndef _SYSCALL_UNOS_HANDLER_
#define _SYSCALL_UNOS_HANDLER_

#include <unostype.h>

GLOBAL USINT64 SyscallHandlerWrapper;
GLOBAL USINT64 SyscallHandler(IN USINT64 SyscallNumber);

#endif