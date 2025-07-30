#ifndef KERNEL_STRING_H
#define KERNEL_STRING_H

#define NULL ((void *)0)

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

#endif
