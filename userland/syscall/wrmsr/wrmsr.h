#ifndef _WRMSR_
#define _WRMSR_

#include <unostype.h>

#define IA32_STAR  0xC0000081
#define IA32_LSTAR 0xC0000082
#define IA32_FMASK 0xC0000084

#ifdef WRMSR_MACRO
    
    #define INUSEMSR
    #define WRMSRSETUP
    #define ASMWRMSR

#endif

UNFUNCTION
SetupSyscallWrmsr(
    IN USINT64 SyscallHandlerAddr
);

#endif