#ifndef _IDT_H_
#define _IDT_H_

#include <stdint.h>

void init_idt();

typedef struct interrupt_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t flags;
    uint64_t rsp;
    uint64_t ss;
} interrupt_frame_t;

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
    uint64_t rip, cs, rflags, rsp, ss;
} registers_t;

uint64_t gettick_handler();
void delay(int ms);

extern volatile uint32_t minute;
extern volatile uint8_t second;

#endif