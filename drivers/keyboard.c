#include <unoskrnl.h>

#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT 0x64

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" 
            : "=a"(ret)
            : "Nd"(port));
    return ret;
}

static char last_char = 0;

char ascii_translator(uint8_t scancode) {
    const char scancode_to_ascii[128] = {
    [0x1E] = 'A', [0x30] = 'B', [0x2E] = 'C', [0x20] = 'D',
    [0x12] = 'E', [0x21] = 'F', [0x22] = 'G', [0x23] = 'H',
    [0x17] = 'I', [0x24] = 'J', [0x25] = 'K', [0x26] = 'L',
    [0x32] = 'M', [0x31] = 'N', [0x18] = 'O', [0x19] = 'P',
    [0x10] = 'Q', [0x13] = 'R', [0x1F] = 'S', [0x14] = 'T',
    [0x16] = 'U', [0x2F] = 'V', [0x11] = 'W', [0x2D] = 'X',
    [0x15] = 'Y', [0x2C] = 'Z',

    [0x0B] = '0', [0x02] = '1', [0x03] = '2', [0x04] = '3',
    [0x05] = '4', [0x06] = '5', [0x07] = '6', [0x08] = '7',
    [0x09] = '8', [0x0A] = '9',

    [0x1C] = '\n',  // Enter
    [0x39] = ' ',   // Spacebar
    [0x0C] = '-',   // -
    [0x0D] = '=',   // =
    [0x1A] = '[',   // [
    [0x1B] = ']',   // ]
    [0x27] = ';',   // ;
    [0x28] = '\'',  // '
    [0x33] = ',',   // ,
    [0x34] = '.',   // .
    [0x35] = '/',   // /
    [0x29] = '`'    // `
};

    if (scancode < 128)
        return scancode_to_ascii[scancode];
    return 0;
}

void keyboard_handler(void* _) {
    uint8_t scancode = inb(0x60);
    apic_send_eoi(); // Kirim EOI ke PIC
    if (scancode & 0x80) {
        // Key release, abaikan aja
    } else {
        last_char = ascii_translator(scancode);
    }

    //serial_print("Keyboard Scancode: 0x");
    //serial_print_hex((uint64_t)scancode);
    //serial_print("\n");

    last_char = ascii_translator(scancode);
}

char keyboard_get_char() {
     while(last_char == 0);

     char c = last_char;
     last_char = 0; 
     return c;
}