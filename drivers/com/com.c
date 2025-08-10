#include <unoskrnl.h>


#define COM1 0x3F8

SINLINE VOID outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

SINLINE USINT8 inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" 
            : "=a"(ret)
            : "Nd"(port));
    return ret;
}

VOID init_serial() {
    outb(COM1 + 1, 0x00); // disable interrupt
    outb(COM1 + 3, 0x80); // aktifkan DLAB
    outb(COM1 + 0, 0x03); // set divisor to 3 (lo byte) 38400 baud
    outb(COM1 + 1, 0x00); //                  (hi byte)
    outb(COM1 + 3, 0x03); // 8 Bits, no parity, one stop hit
    outb(COM1 + 2, 0xC7); // ENABLe FIFO, clear them with 14 bytes threshold
    outb(COM1 + 4, 0x0B); // IRQs Enabled
}

INT is_transit_empty() {
    return inb(COM1 + 5) & 0x20;
}

VOID serial_write_char (char a) {
    while(is_transit_empty() == 0); 
    outb(COM1, a);
    //if(GraphicOK == 1) PSFPutChar(a, defaultcolor);
}

VOID serial_print(const char *str) {
    while(*str) {
        if(*str == '\n') serial_write_char('\r');
        serial_write_char(*str);
        str++;
    }
}

VOID serial_print_hex(uint64_t value) {
    CHARA8 hex[] = "0123456789ABCDEF";
    CHARA8 buffer[17];
    buffer[16] = '\0'; 

    for (INT i = 15; i >= 0; i--) {
        buffer[i] = hex[value & 0xF];
        value >>= 4;
    }

    serial_print("0x");
    serial_print(buffer);
}

VOID serial_printf(const char *fmt, ...) {
    va_list args;
    va_start (args, fmt);

    while(*fmt) {
        if(*fmt == '%') {
            fmt++;
            //Parse flag
            BOOL zero_pad = FALSE;
            if (*fmt == '0') {
                zero_pad = TRUE;
                fmt++;
            }

            //parse width
            INT width = 0;
            while(*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }

            CHARA8 buf[32];
            const char *str = NULL;

            switch (*fmt) {
                case 's':
                    {str = va_arg(args, const char*);
                    serial_print(str);
                    break;}
                case 'c':
                    {serial_write_char((char)va_arg(args, int));
                    break;}
                case 'd':
                case 'i':
                    {int val = va_arg(args, int);
                    itoa(val, buf);
                    int len = strlen(buf);
                    for (int i = len; i < width; i++) serial_write_char(zero_pad ? '0' : ' ');
                    serial_print(buf);
                    break;}
                case 'u':
                    {unsigned int val = va_arg(args, unsigned int);
                    utoa(val, buf);
                    int len = strlen(buf);
                    for (int i = len; i < width; i++) serial_write_char(zero_pad ? '0' : ' ');
                    serial_print(buf);
                    break;}
                case 'x':
                    {int uppercase = (*fmt == 'X');
                    uint64_t val = va_arg(args, uint64_t);
                    xtoa(val, buf, uppercase);
                    int len = strlen(buf);
                    for (int i = len; i < width; i++) serial_write_char(zero_pad ? '0' : ' ');
                    serial_print(buf);
                    break;}
                case 'X':
                    {int uppercase = (*fmt == 'X');
                    uint64_t val = va_arg(args, uint64_t);
                    xtoa(val, buf, uppercase);
                    int len = strlen(buf);
                    for (int i = len; i < width; i++) serial_write_char(zero_pad ? '0' : ' ');
                    serial_print(buf);
                    break;}
                case 'p':
                    {VOID *ptr = va_arg(args, VPTR);
                    xtoa((uint64_t)(uintptr_t)ptr, buf, 0);
                    serial_print("0x");
                    serial_print(buf);
                    break;}
                case 'l':
                    if (*(fmt + 1) == 'l' && *(fmt + 2) == 'u') {
                        fmt += 2;
                        unsigned long long val = va_arg(args, unsigned long long);
                        utoa(val, buf);
                        serial_print(buf);
                    } else {
                        serial_write_char('%');
                        serial_write_char('l');
                        serial_write_char(*(fmt + 1));
                        fmt+= 2;
                        continue;
                    }
                break;
                case '%':
                    serial_write_char('%');
                    break;
                default:
                    serial_write_char('%');
                    serial_write_char(*fmt);
                    
            }
        } else {
            serial_write_char(*fmt);
        }
        fmt++;
    }
    va_end(args);
}

VOID serial_vprintf(const char *fmt, va_list args) {
    while(*fmt) {
        if(*fmt == '%') {
            fmt++;
            //Parse flag
            BOOL zero_pad = FALSE;
            if (*fmt == '0') {
                zero_pad = TRUE;
                fmt++;
            }

            //parse width
            INT width = 0;
            while(*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }

            CHARA8 buf[32];
            const char *str = NULL;

            switch (*fmt) {
                case 's':
                    {str = va_arg(args, const char*);
                    serial_print(str);
                    break;}
                case 'c':
                    {serial_write_char((char)va_arg(args, int));
                    break;}
                case 'd':
                case 'i':
                    {int val = va_arg(args, int);
                    itoa(val, buf);
                    int len = strlen(buf);
                    for (int i = len; i < width; i++) serial_write_char(zero_pad ? '0' : ' ');
                    serial_print(buf);
                    break;}
                case 'u':
                    {unsigned int val = va_arg(args, unsigned int);
                    utoa(val, buf);
                    int len = strlen(buf);
                    for (int i = len; i < width; i++) serial_write_char(zero_pad ? '0' : ' ');
                    serial_print(buf);
                    break;}
                case 'x':
                    {int uppercase = (*fmt == 'X');
                    uint64_t val = va_arg(args, uint64_t);
                    xtoa(val, buf, uppercase);
                    int len = strlen(buf);
                    for (int i = len; i < width; i++) serial_write_char(zero_pad ? '0' : ' ');
                    serial_print(buf);
                    break;}
                case 'X':
                    {int uppercase = (*fmt == 'X');
                    uint64_t val = va_arg(args, uint64_t);
                    xtoa(val, buf, uppercase);
                    int len = strlen(buf);
                    for (int i = len; i < width; i++) serial_write_char(zero_pad ? '0' : ' ');
                    serial_print(buf);
                    break;}
                case 'p':
                    {VOID *ptr = va_arg(args, VPTR);
                    xtoa((uint64_t)(uintptr_t)ptr, buf, 0);
                    serial_print("0x");
                    serial_print(buf);
                    break;}
                case 'l':
                    if (*(fmt + 1) == 'l' && *(fmt + 2) == 'u') {
                        fmt += 2;
                        unsigned long long val = va_arg(args, unsigned long long);
                        utoa(val, buf);
                        serial_print(buf);
                    } else {
                        serial_write_char('%');
                        serial_write_char('l');
                        serial_write_char(*(fmt + 1));
                        fmt+= 2;
                        continue;
                    }
                break;
                case '%':
                    serial_write_char('%');
                    break;
                default:
                    serial_write_char('%');
                    serial_write_char(*fmt);
                    
            }
        } else {
            serial_write_char(*fmt);
        }
        fmt++;
    }
}


/* END COM CODE*/