[BITS 64]

global isr0
global isr1
global isr13
global isr14
global isr32
global isr33
global irq_common_stub
global isr_mouse
global syscall_handler_wrapper
extern isr0_handler_c
extern isr1_handler_c
extern isr13_handler_c
extern isr14_handler_c
extern timer_handler
extern irq_handler_c
extern dump_regs
extern keyboard_handler
extern MouseInterrupt
extern SyscallHandler

; ISR COMMON STUB FOR iretq

global irq_common_stub
irq_common_stub:
    ; Dummy push buat align
    push rax

    ; Save regs
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rsi
    push rdi
    push rbp
    push rdx
    push rcx
    push rbx
    push rax

    ; STACK SEKARANG offset 0 (128+16 = 144)
    ; Tapi kita mau SEBELUM call C FUNC offsetnya 0,
    ; jadi kita SUB RSP 8 untuk bikin 16-byte alignment pas `call`
    sub rsp, 8

    call irq_handler_c

    add rsp, 8  ; cleanup padding

    ; Restore regs
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rbp
    pop rdi
    pop rsi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    pop rax   ; dummy

    iretq



; THE ISR

isr0:
    cli
    push rax
    push rcx
    push rdx
    call isr0_handler_c
    pop rdx
    pop rcx
    pop rax
    sti
    iretq

isr1:
    cli
    push rax
    push rcx
    push rdx
    mov rdi, [rsp + 24]
    call isr1_handler_c
    pop rdx
    pop rcx
    pop rax
    sti
    iretq

isr13:
    cli
    mov rdi, rsp
    mov  rsi, [rsp + 40]
    sub rsp, 8
    call isr13_handler_c
    add rsp, 8
    iretq

isr14:
    cli
    push rax
    push rcx
    push rdx
    mov rdi, [rsp + 24]        ; Ambil error_code
    call isr14_handler_c
    pop rdx
    pop rcx
    pop rax
    add rsp, 8                 ; Buang error_code dari stack
    sti
    iretq

isr32:
    ; Push interrupt stack frame secara manual
    push qword 0     ; error code dummy (jika tidak ada)
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rsi
    push rdi
    push rbp
    push rdx
    push rcx
    push rbx
    push rax

    mov rdi, rsp              ; IRQ number pointer to current stack frame
    call irq_handler_c

    ; Pop all general-purpose registers
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rbp
    pop rdi
    pop rsi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    add rsp, 8                ; buang dummy error code

    iretq

isr33:
    ; Push interrupt stack frame secara manual
    cld
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rsi
    push rdi
    push rbp
    push rdx
    push rcx
    push rbx
    push rax

    mov rdi, rsp              ; IRQ number pointer to current stack frame
    call keyboard_handler

    ; Pop all general-purpose registers
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rbp
    pop rdi
    pop rsi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    iretq

isr_mouse:
        ; Push interrupt stack frame secara manual
    cld
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rsi
    push rdi
    push rbp
    push rdx
    push rcx
    push rbx
    push rax

    mov rdi, rsp              ; IRQ number pointer to current stack frame
    call MouseInterrupt
    

    ; Pop all general-purpose registers
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rbp
    pop rdi
    pop rsi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    iretq

syscall_handler_wrapper:
; This is the syscall handler wrapper
    ; Push interrupt stack frame secara manual
    push rax
    push rdi
    push rsi
    push rdx

    ;SYSCALL CONVENTION:
    ; SyscallHandler(rax, rdi, rsi, rdx)
    mov rdi, [rsp + 24] ; rax
    mov rsi, [rsp + 16] ; rdi
    mov rdx, [rsp + 8] ; rsi
    mov rcx, [rsp + 0] ; rdx

    call SyscallHandler
    pop rdx
    pop rsi
    pop rdi
    pop rax

    iretq



