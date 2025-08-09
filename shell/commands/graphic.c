#include <unoskrnl.h>

UNFUNCTION Graphic(CONST CHARA8 *arg) {
    if (arg == NULL || *arg == '\0') {
        serial_print("GRAPHIC: 0: Use Serial. 1: Use Framebuffer\n");
        return;
    }

    if(*arg == '0') {
        GraphicEnabled = 0;
        ResetCursorX();
        ResetCursorY();
        drawfullscreen(0xFF000000);
        init_terminal();
    } else if (*arg == '1') {
        drawfullscreen(0xFF000000);
        GraphicEnabled = 1;
        ResetCursorX();
        ResetCursorY();
        GraphicTerminalINIT();
    }
}