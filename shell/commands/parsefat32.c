#include <unoskrnl.h>

void parsefat32(const char *arg) {
    serial_printf("Parsing FAT32...\n");
    ParseFAT32(ahci_port);

    serial_printf("Here are the root directories found on your FAT32 partition.\n");
    serial_printf("To open a subdirectory, use the command: CD <folder cluster number>\n");
}

void cd(const char *arg) {
    if (arg == NULL || *arg == '\0') {
        serial_print("Type a number between 1 - 3. And see the magic!!\n");
        return;
    }
    int sector = atoi(arg);

    ReadDirectory(ahci_port, sector);
}