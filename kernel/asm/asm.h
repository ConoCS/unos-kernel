#ifndef _ASM_
#define _ASM_

#include <unostype.h>

typedef struct {
    uint64_t rip, rsp, rflags;
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    // Bisa disesuaikan sesuai arsitektur dan cara kamu simpan register saat interrupt
} CPU_Registers;

UNFUNCTION TakeRegister(OUT CPU_Registers* regs);
void serial_print_registers(CPU_Registers *regs);

#endif