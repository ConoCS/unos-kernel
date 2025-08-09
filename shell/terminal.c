#include <unoskrnl.h>

char charbuffer[512];
INT GraphicEnabled = 0;

typedef void(*CommandFunc)(const char *arg);
typedef struct {
    char *name;
    CommandFunc function;
} Command;

void run_command(char *input) {
    char *command = strtok(input, " ");
    char *arg = strtok(NULL, "\n");

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

    int num_commands = sizeof(commands) / sizeof(commands[0]);

    for(int i = 0; i < num_commands; i++) {
        if(strcmp(command, commands[i].name) == 0) {
            commands[i].function(arg);
            return;
        }
    }

    serial_print(command);
    serial_print(" ");
    serial_print(" Is not a valid command nor a valid operable program");
    serial_print("\n");
}

void terminal_queue_word() {
    int index = 0;

    while (1) {
        char c = keyboard_get_char();
        serial_write_char(c);
        if (c == '\n') {
            charbuffer[index] = '\0';  // akhir string
            serial_print("\n");

            // Eksekusi pseudo: bisa ganti nanti jadi parsing command
            run_command(charbuffer);

            index = 0;  // reset buffer
            serial_print("kernelland_serial@unos $ ");
        } else {
            if (index < 511) {
                charbuffer[index++] = c;
            }
        }
    
    }
}

void init_terminal() {
    serial_print("Copyright UNT University 2025\n");
    serial_print("UnOS Minimalist Terminal v0.09ah\n\n");

    serial_print("kernelland_serial@unos $ ");
    terminal_queue_word();
}

