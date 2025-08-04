#ifndef KERNEL_STRING_H
#define KERNEL_STRING_H

#include <unostype.h>

typedef unsigned long size_t;

int strcmp(const char *s1, const char *s2);
char *strtok(char *str, const char *delim);
int strlen(const char *str);
char *strchr(const char *str, int c);
int atoi(const char *str);
void itoa(int value, char *buffer);
void utoa(uint64_t value, char *buffer);
void xtoa(uint64_t value, char *buffer, int uppercase);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy (void *dest, const void *src, size_t n);
unsigned long strtoul(const char *nptr, char **endptr, int base);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
int strcasecmp(const char *a, const char *b);
CHARA8 toupper(CHARA8 ch) ;

#endif
