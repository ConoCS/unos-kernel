#include <unoskrnl.h>

/**
 * @brief A more complex function of serial_print and PSFPutChar.
 * Print a message to the screen also with level and type of info
 * 
 */
UNFUNCTION Printk(printk_level_t level, const char *fmt, ...) {
    const char* prefix;
    switch(level) {
        case KINFO: prefix = "[INFO] "; break;
        case KWARN: prefix = "[WARNING] "; break;
        case KSUCCESS: prefix = "[ OK ] "; break;
        case KERR: prefix = "[ERROR] "; break;
        default: prefix = "[UNK] "; break;
    }

    serial_printf("%s", prefix);

    va_list args;
    va_start(args, fmt);
    serial_vprintf(fmt, args);
    va_end(args);
}