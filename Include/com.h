#pragma once
#include <stdint.h>

void serial_print(const char *str);
void serial_print_hex(uint64_t value);
void serial_write_char (char a);
void serial_printf(const char *fmt, ...);