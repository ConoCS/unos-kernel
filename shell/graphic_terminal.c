#include <unoskrnl.h>

char CharbufferG[512];

typedef void(*CommandFunc)(const char *arg);
typedef struct {
    char *name;
    CommandFunc function;
} Command;

UNFUNCTION
RunCommandG(
    CHARA8 *input
)
{
    CHARA8 *command = strtok(input, " ");
    CHARA8 *arg = strtok(NULL, "\n");

    if(!command) return;

    Command commands[] = {
        {"ECHO", echo},
        {"PARTY", party},
        {"HELP", help},
        {"GETTICK", gettick},
        {"WAITFORTICK", waitfortick},
        {"TRYBUFFER", trybuffer},
        {"UPTIME", uptime},
        {"PARSEFAT32", parsefat32},
        {"CD", cd},
        {"SHUTDOWN", Shutdown},
        {"SLEEP", Sleep},
        {"GRAPHIC", Graphic}
    };

    INT NumOfCommands = sizeof(commands) / sizeof(commands[0]);

    for(INT i = 0; i < NumOfCommands; i++) {
        if(strcmp(command, commands[i].name) == 0) {
            commands[i].function(arg);
            return;
        }
    }

    PSFPrintf(command);
    PSFPrintf(" Is not an operable program or command\n");
}

UNFUNCTION
GraphicTerminalQueueWord(
)
{
    int index = 0;

    while(1) {
        char c = keyboard_get_char();
        PSFPutChar(c, defaultcolor);
        if (c == '\n') {
            CharbufferG[index] = '\0';

            RunCommandG(CharbufferG);

            index = 0;
            PSFPutChar('\n', defaultcolor);
            DrawSimplePSFText("kernelland_unosgrphc@unos $ ", defaultcolor);
        } else {
            if(index < 511) {
                CharbufferG[index++] = c;
            }
        }
    }
}

FUNCWITHSTATUS
GraphicTerminalINIT(

)
{
    GraphicEnabled = 1;
    DrawSimplePSFText("Copyright UnOS Team C 2025\n", defaultcolor);
    DrawSimplePSFText("UnOS Terminal v0.12ah Kernel-land\n\n", defaultcolor);

    DrawSimplePSFText("kernelland_unosgrphc@unos $ ", defaultcolor);
    GraphicTerminalQueueWord();
    return STATUS_OK;
}