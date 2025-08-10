#ifndef UNOS_H
#define UNOS_H

// Core Types & Macros
#include <unostype.h>
#include <kerrno.h>

// Kernel Core
#include "kernel/bootinfo.h"
#include "kernel/pic/pic.h"

// Memory & Paging
#include "kernel/memory/memory.h"

// Interrupts
#include "kernel/idt/idt.h"

// Drivers
#include "drivers/keyboard.h"
#include "drivers/pci/pci.h"
#include "drivers/storage/storage.h"
#include "drivers/storage/fat32.h"
#include "drivers/storage/gpt.h"
#include "drivers/watchdog/wdsoft.h"

// ACPI
#include "acpi/acpi.h"
#include "acpi/fadt.h"
#include "acpi/madt.h"
#include "acpi/bgrt.h"

// Filesystem & Extensions
#include "fs/vfs.h"
#include "fs/spinlock.h"
#include "fs/ext/psf.h"
#include "fs/ext/elf.h"

// Graphics
#include "graphics/graphic.h"

// Scheduler & Timer
#include "kernel/scheduler/scheduler.h"
#include "kernel/scheduler/schetask.h"
#include "kernel/scheduler/tasklist.h"
#include "kernel/timer/pit.h"

// Shell & Terminal
#include "shell/terminal.h"
#include "shell/command.h"

// Userland
#include "userland/userland.h"
#include "userland/syscall/syscall.h"
#include "userland/syscall/wrmsr/wrmsr.h"

// Utilities and PANIC LOGGING
#include <kstring.h>    // only the header, not .c!
#include <com.h>
#include "kernel/logging/printk/printk.h"
#include "kernel/logging/panic/panic.h"

// ASM Related function
#include "kernel/asm/asm.h"

#endif // UNOS_H
