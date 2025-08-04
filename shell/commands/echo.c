#include <unoskrnl.h>

void echo(const char *string) {
    if (!string || string[0] == '\0') {
        serial_print("ECHO is on\n");
        return;
    }

    if (strcmp(string, "KONTOL") == 0) {
        serial_print("UnOS: Gaboleh toxic yahhh :)\n");
        return;
    }

    if (strcmp(string, "RING") == 0) {
        serial_print("RING Status: RING 0 as Kernel-land\n");
        return;
    }

    serial_print(string);
    serial_print("\n");
}
