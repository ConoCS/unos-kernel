ENTRY(_start)

SECTIONS {
  . = 0x200000;

  _kernel_start = .;

  .text : ALIGN(0x1000) {
    KEEP(*(.text.boot))
    KEEP(*(.text*))
  }

  .rodata : ALIGN(0x1000) { KEEP(*(.rodata*)) }
  .data : ALIGN(0x1000) { KEEP(*(.data*)) }
  .bss : ALIGN(0x1000) {
    KEEP(*(.bss*))
    *(COMMON)
  }

  _kernel_end = .;
}
