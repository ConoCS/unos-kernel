#ifndef _UNOSKRNL_H_
#define _UNOSKRNL_H_

#include <unostype.h>

#include "com.h"
#include "graphic.h"
#include "kerrno.h"
#include "paging.h"
#include "scheduler.h"
#include "string.h"

#include "../acpi/acpi.h"
#include "../acpi/madt.h"

#include "../boot/bootinfo.h"

#include "../driver/pci/pci.h"
#include "../driver/keyboard.h"
#include "../driver/storage.h"
#include "../driver/storage/fat32.h"
#include "../driver/storage/gpt.h"

#include "../file/extension/elf.h"
#include "../file/extension/psf.h"

#include "../filesystem/vfs.h"

#include "../idt/errornumber/errnum.h"
#include "../idt/idt.h"

#include "../pic/pic.h"

#include "../shceduler/schetask.h"
#include "../shceduler/tasklist.h"

#include "../terminal/command.h"
#include "../terminal/terminal.h"

#include "../timer/pit.h"




#endif