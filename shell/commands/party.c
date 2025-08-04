#include <unoskrnl.h>

void party(const char *wowo) {
    if (wowo == NULL || *wowo == '\0') {
        serial_print("Type a number between 1 - 3. And see the magic!!\n");
        return;
    }
    int code = atoi(wowo);

    serial_print("Want to party? LETS GOOOO\n");
    switch(code) {
        case 1:
        serial_print("This one is PAGE FAULT\n");
        volatile int *pointercrash = (int *)(uintptr_t)0xDEADBEEF;
        *pointercrash = 0x1337;
        break;
        case 2:
        serial_print("This one is DIVIDE BY ZERO\n");
        int a = 1;
        int b = 0;
        int c = a / b;
        break;
        case 3:
        serial_print("This one is GP\n");
        volatile int *idk = (int *)(uintptr_t)0xDEADBEEFCAFEBABE;
        *idk = 0xB00B135;
        break;
        default:
        serial_print("Type a number between 1 - 3. And see the magic!!\n");
    }
}