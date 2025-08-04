#include <unoskrnl.h>

void gettick(const char *arg) {
    uint64_t tickofnow = gettick_handler();
    serial_print("Current kernel tick in HEXA: "); serial_print_hex(tickofnow);
    serial_print("\n");
    serial_printf("Curren kernel tick in DECIMAL: %d\n", tickofnow);
    serial_printf("Watchdog TICK: %d\n", WatchdogUnOSKrnl->counter);
}