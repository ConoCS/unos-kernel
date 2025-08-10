#include <unoskrnl.h>

UNFUNCTION
TakeRegister(OUT CPU_Registers* regs) {

}

void serial_print_registers(CPU_Registers *regs) {
    serial_printf("RIP:     0x%016llx\n", regs->rip);
    serial_printf("RSP:     0x%016llx\n", regs->rsp);
    serial_printf("RFLAGS:  0x%016llx\n", regs->rflags);
    serial_printf("RAX:     0x%016llx\n", regs->rax);
    serial_printf("RBX:     0x%016llx\n", regs->rbx);
    serial_printf("RCX:     0x%016llx\n", regs->rcx);
    serial_printf("RDX:     0x%016llx\n", regs->rdx);
    serial_printf("RSI:     0x%016llx\n", regs->rsi);
    serial_printf("RDI:     0x%016llx\n", regs->rdi);
    serial_printf("RBP:     0x%016llx\n", regs->rbp);
    serial_printf("R8:      0x%016llx\n", regs->r8);
    serial_printf("R9:      0x%016llx\n", regs->r9);
    serial_printf("R10:     0x%016llx\n", regs->r10);
    serial_printf("R11:     0x%016llx\n", regs->r11);
    serial_printf("R12:     0x%016llx\n", regs->r12);
    serial_printf("R13:     0x%016llx\n", regs->r13);
    serial_printf("R14:     0x%016llx\n", regs->r14);
    serial_printf("R15:     0x%016llx\n", regs->r15);
}
