BITS 64

section .text.boot
global _start
extern KernelMain

_start:
lgdt [gdt_descriptor]
    ; kosongin register
    xor rax, rax
xor rbx, rbx
xor rcx, rcx
xor rdx, rdx
xor rsi, rsi
xor r8, r8
xor r9, r9
xor r10, r10
xor r11, r11
xor r12, r12
xor r13, r13
xor r14, r14
xor r15, r15

    ; isi register dengan data penting. 0x10 as data segment GDT
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax  
    
    mov rsp, stack_top
    mov rbp, rsp

    ; Far return to load CS = 0x08
    push 0x08
    lea rax, [rel real_kernel_entry]
    push rax
    retfq

real_kernel_entry:
    call KernelMain

hang:
    hlt
    jmp hang



; -- DATA DESCRIPTOR --

align 8
gdt_start:
    dq 0x0000000000000000 ; Null Descriptor
    dq 0x00AF9A000000FFFF ; Code Segment 64 BITS
    dq 0x00AF92000000FFFF ; Data Segment

gdt_descriptor:
    dw gdt_descriptor_end - gdt_start - 1
    dq gdt_start

gdt_descriptor_end:

section .bss
align 16
stack_bottom:
    resb 8192
stack_top:

