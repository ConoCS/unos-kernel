#include <unoskrnl.h>

VOID Shutdown(CONST IN CHARA8 *arg) {
    serial_printf("Shutdown command received with argument: %s\n", arg);
    
    // Check if ACPI is available
    if (g_S5Info.Found) {
        AcpiShutdown();
    } else {
        serial_printf("[Error] ACPI not available, cannot shutdown.\n");
    }

    // Hang the system
    while(1) {
        asm("hlt");
    }
}