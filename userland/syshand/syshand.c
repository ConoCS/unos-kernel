#include <unostype.h>
#include <unoskrnl.h>

VOID SYSCALLWrite(USINT64 str_ptr) {
    CONST CHARA8 *str = (CHARA8*)str_ptr;
    while (*str) {
        if (*str == '\n') serial_write_char('\r');
        serial_write_char(*str++);
    }
}

VOID SYSCALLExit() {
    serial_printf("[Userland] Program exited!\n");
    while(1) __asm__("hlt");
}

VPTR SYSCALLTable[SYSCALL_COUNT] = {
    [SYSCALL_WRITE] = SYSCALLWrite,
    [SYSCALL_EXIT] = SYSCALLExit,
};

VOID SyscallHandler(USINT64 RAX, USINT64 RDI, USINT64 RSI, USINT64 RDX) {
    if (RAX >= SYSCALL_COUNT) return;

    VPTR fn = SYSCALLTable[RAX];
    if (!fn) return;

    ((VOID (*)(USINT64, USINT64, USINT64))fn)(RDI, RSI, RDX);
}
