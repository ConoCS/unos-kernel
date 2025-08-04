#include <unoskrnl.h>

void help(const char *helpcommand) {
    serial_print("List operable of command:\n\n");

    serial_print("ECHO: Print user string or requested data to terminal\n");
    serial_print("GETTICK: Get current kernel tick for now\n");
    serial_print("HELP: Show help\n");
    serial_printf("PARSEFAT32: Parse FAT32 and show folders and files inside the first partition\n");
    serial_print("PARTY: Trigger Kernel Panic \n");
    serial_print("TRYBUFFER: Printing a buffer based on a user count input\n");
    serial_print("UPTIME: Check Kernel lifetime\n");
    serial_print("WAITFORTICK: Make terminal waiting for a requested tick \n\n");

    serial_print("System ACPI:\n");
    serial_print("SHUTDOWN: Shutdown the system using ACPI _S5 method\n\n");
    serial_print("SLEEP: Sleep the system using ACPI _S5 method\n\n");


    serial_print("For more command arguments info, type HELP <command>\n");
}