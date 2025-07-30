#include "../../Include/com.h"
#include "../terminal.h"
#include "../command.h"
#include "../../Include/string.h"
#include "../../idt/idt.h"

void gettick(const char *arg) {
    uint64_t tickofnow = gettick_handler();
    serial_print("Current kernel tick in HEXA: "); serial_print_hex(tickofnow);
    serial_print("\n");
    serial_printf("Curren kernel tick in DECIMAL: %d\n", tickofnow);
}