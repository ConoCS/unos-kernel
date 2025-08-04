#pragma once
#include <stdint.h>

// ANSI color escape codes
#define ANSI_COLOR_RESET   "\x1b[0m"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"


// markdown style of ANSI escape
#define ANSI_BOLD_ON       "\x1b[1m"
#define ANSI_BOLD_OFF      "\x1b[22m"


void serial_print(const char *str);
void serial_print_hex(uint64_t value);
void serial_write_char (char a);
void serial_printf(const char *fmt, ...);