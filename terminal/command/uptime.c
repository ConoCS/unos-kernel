#include "../../Include/com.h"
#include "../terminal.h"
#include "../command.h"
#include "../../Include/string.h"
#include "../../idt/idt.h"

void uptime(const char *arg) {
    uint8_t localsecond = second;
    uint32_t localminute = minute;

    serial_printf("UPTIME: %d Minute %d Second (MM:SS)\n", localminute, localsecond);
}