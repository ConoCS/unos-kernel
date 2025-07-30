#include "../../Include/com.h"
#include "../terminal.h"
#include "../command.h"
#include "../../Include/string.h"

void help(const char *helpcommand) {
    serial_print("List operable of command:\n\n");

    serial_print("ECHO: Print user string or requested data to terminal\n");
    serial_print("GETTICK: Get current kernel tick for now\n");
    serial_print("HELP: Show help\n");
    serial_print("PARTY: Trigger Kernel Panic \n");
    serial_print("WAITFORTICK: Make terminal waiting for a requested tick \n\n");


    serial_print("For more command arguments info, type HELP <command>\n");
}