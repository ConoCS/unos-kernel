#include <unoskrnl.h>

void waitfortick(const char *arg) {
    if (arg == NULL || *arg == '\0') {
        serial_print("WAITFORTICK: No requested tick inputted\n");
        return;
    }
    uint64_t tick_wait = (uint64_t)atoi(arg);
    serial_print("Waiting for requested tick...\n");

    while (1) {
        if(tick_wait < gettick_handler()) {
            serial_print("The kernel tick has already passed the tick you requested\n");
            serial_print("Use GETTICK Command to get the information of current tick\n");
            break;
        }
        if(tick_wait == gettick_handler()) {
            serial_print("Matching requested tick!\n");
            break;
        }
    }
}