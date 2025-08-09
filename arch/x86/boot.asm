BITS 64

section .text.boot
global _start
extern KernelMain

; selector untuk TSS (index ke-5 * 8)
%define TSS_SELECTOR (5 * 8)

_start:
    ; Load GDT (GDT sudah ada di data section)
    lgdt [gdt_descriptor]

    ; --- isi TSS.rsp0 (stack untuk ring0) ---
    ; lea mendapatkan alamat tss_entry RIP-relative (64-bit)
    lea rax, [rel kernel_stack_top]
    ; simpan alamat stack kernel ke RSP0 pada tss_entry (offset 4 dalam TSS64)
    mov qword [tss_entry + 4], rax

    ; --- Build TSS descriptor (2 qword) di runtime ---
    ; rax = base address of tss_entry
    lea rax, [rel tss_entry]

    ; limit = sizeof(TSS) - 1 = 104 - 1
    mov rcx, 104 - 1        ; rcx = limit

    ; rcx_low = limit & 0xFFFF
    mov rdx, rcx
    and rdx, 0xFFFF

    ; desc_low starts with limit_low
    mov r8, rdx             ; r8 = desc_low partial

    ; base_low16 -> (base & 0xFFFF) << 16
    mov r9, rax
    and r9, 0xFFFF
    shl r9, 16
    or r8, r9

    ; base_mid8 -> ((base >> 16) & 0xFF) << 32
    mov r9, rax
    shr r9, 16
    and r9, 0xFF
    shl r9, 32
    or r8, r9

    ; access byte 0x89 -> placed at bits 40..47
    mov r9, 0x89
    shl r9, 40
    or r8, r9

    ; limit_hi4 (limit >> 16) & 0xF -> bits 48..51
    mov r9, rcx
    shr r9, 16
    and r9, 0xF
    shl r9, 48
    or r8, r9

    ; flags (bits 52..55) -> 0x0 for TSS
    ; (no need to OR if zero)

    ; base_high8 -> ((base >> 24) & 0xFF) << 56
    mov r9, rax
    shr r9, 24
    and r9, 0xFF
    shl r9, 56
    or r8, r9

    ; r8 sekarang = low 64-bit descriptor
    mov [tss_descriptor], r8

    ; high qword = base >> 32 in low 32 bits, rest zero
    mov r9, rax
    shr r9, 32
    and r9, 0xFFFFFFFF
    mov [tss_descriptor + 8], r9

    ; --- load TSS selector ---
    mov ax, TSS_SELECTOR
    ltr ax

    ; clear registers (optional)
    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx
    xor rsi, rsi
    xor r8,  r8
    xor r9,  r9
    xor r10, r10
    xor r11, r11
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15

    ; Set data segment ke 0x10 (kernel data)
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Set stack
    lea rsp, [rel kernel_stack_top]
    mov rbp, rsp

    ; Far return untuk load CS = 0x08 (kernel code)
    push 0x08
    lea rax, [rel real_kernel_entry]
    push rax
    retfq

real_kernel_entry:
    call KernelMain

hang:
    hlt
    jmp hang


; ===== GDT =====
section .data
align 8
gdt_start:
    dq 0x0000000000000000             ; Null
    dq 0x00AF9A000000FFFF             ; Kernel Code
    dq 0x00AF92000000FFFF             ; Kernel Data
    dq 0x00AFFA000000FFFF             ; User Code (DPL=3)
    dq 0x00AFF2000000FFFF             ; User Data (DPL=3)

    ; TSS descriptor placeholder (2 qwords). kita akan overwrite di runtime.
tss_descriptor:
    dq 0x0000000000000000
    dq 0x0000000000000000

gdt_descriptor:
    dw gdt_descriptor_end - gdt_start - 1
    dq gdt_start
gdt_descriptor_end:

; ===== BSS =====
section .bss
align 16
tss_entry:
    resb 104        ; 64-bit TSS size

align 16
kernel_stack_bottom:
    resb 8192
kernel_stack_top:
