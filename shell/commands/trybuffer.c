#include <unoskrnl.h>

void trybuffer(const char *argcount) {
    if (argcount == NULL || *argcount== '\0') {
        serial_print("TRYBUFFER: please type a number\n");
        return;
    }

    kerrno = 0;
    char *endptr;
    unsigned long count = strtoul(argcount, &endptr, 10);
    if(kerrno == ERANGE || *endptr != '\0') {
        serial_printf("TRYBUFFER: Invalid number\n");
        return;
    }

    if (count > 10000) {
        serial_printf("TRYBUFFER: Number too big\n");
        return;
    }

    uint32_t *buffer = (uint32_t*)kmalloc(count * sizeof(uint32_t));
    if(!buffer) {
        serial_printf("[Fatal Error] KMalloc error. Please makefile AGAIN!!\n");
    }

    for(uint32_t i = 0; i < count; i++) {
        buffer[i] = i * 10;
        serial_printf("Buffer[%d] contains data of: %d\n", i, buffer[i]);
    }
}