#include <unoskrnl.h>

VOID MouseInterrupt() {
    // Mouse interrupt handler
    // This function will be called when a mouse event occurs
    // You can implement your mouse handling logic here
    serial_print("Mouse interrupt received\n");
    apic_send_eoi();
}