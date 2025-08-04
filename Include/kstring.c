#include <unoskrnl.h>

// Kalau belum ada errno sendiri
int kerrno = 0;
#define ERANGE 34

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strlen(const char *str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

char *strchr(const char *str, int c) {
    while (*str) {
        if (*str == (char)c) {
            return (char *)str;
        }
        str++;
    }

    // cek karakter null di akhir
    if (c == '\0') {
        return (char *)str;
    }

    return NULL;
}


// strtok versi super basic (non-threadsafe)
char *strtok(char *str, const char *delim) {
    static char *saved;
    if (str) saved = str;
    if (!saved) return NULL;

    char *start = saved;
    while (*start && strchr(delim, *start)) start++;
    if (!*start) return NULL;

    char *end = start;
    while (*end && !strchr(delim, *end)) end++;

    if (*end) {
        *end = '\0';
        saved = end + 1;
    } else {
        saved = NULL;
    }

    return start;
}


char* strcpy(char* dest, const char* src) {
    char* ret = dest;
    while ((*dest++ = *src++));  // copy sampai '\0' juga ikut
    return ret;
}

char* strcat(char* dest, const char* src) {
    char* ptr = dest;

    // Geser ptr ke akhir string dest
    while (*ptr != '\0') {
        ptr++;
    }

    // Salin src ke akhir dest
    while (*src != '\0') {
        *ptr++ = *src++;
    }

    // Tambahkan null-terminator
    *ptr = '\0';

    return dest;
}

static int to_lower(int c) {
    if (c >= 'A' && c <= 'Z') return c - 'A' + 'a';
    return c;
}

int strcasecmp(const char *a, const char *b) {
    while (*a && *b) {
        int ca = to_lower((unsigned char)*a++);
        int cb = to_lower((unsigned char)*b++);
        if (ca != cb) return ca - cb;
    }
    return to_lower((unsigned char)*a) - to_lower((unsigned char)*b);
}

int atoi(const char *str) {
    int res = 0;
    while (*str >= '0' && *str <= '9') {
        res = res * 10 + (*str - '0');
        str++;
    }
    return res;
}

void itoa(int value, char* buffer) {
    char tmp[12];
    int i = 0, is_negative = 0;

    if (value == 0) {
        buffer[0] = '0'; buffer[1] = '\0';
        return;
    }

    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    while (value && i < 11) {
        tmp[i++] = '0' + (value % 10);
        value /= 10;
    }

    if (is_negative) tmp[i++] = '-';

    int j = 0;
    while (i--) buffer[j++] = tmp[i];
    buffer[j] = '\0';
}

void utoa(uint64_t value, char* buffer) {
    char tmp[21];
    int i = 0;

    if (value == 0) {
        buffer[0] = '0'; buffer[1] = '\0';
        return;
    }

    while (value && i < 20) {
        tmp[i++] = '0' + (value % 10);
        value /= 10;
    }

    int j = 0;
    while (i--) buffer[j++] = tmp[i];
    buffer[j] = '\0';
}

void xtoa(uint64_t value, char* buffer, int uppercase) {
    const char* hex = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[17];
    int i = 0;

    if (value == 0) {
        buffer[0] = '0'; buffer[1] = '\0';
        return;
    }

    while (value && i < 16) {
        tmp[i++] = hex[value & 0xF];
        value >>= 4;
    }

    int j = 0;
    while (i--) buffer[j++] = tmp[i];
    buffer[j] = '\0';
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;
    while (n--) {
        *p++ = (uint8_t)c;
    }
    return s;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return (int)p1[i] - (int)p2[i];
        }
    }

    return 0;
}

void *memcpy (void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char*) dest;
    const unsigned char *s = (const unsigned char*) src;

    for(size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}

unsigned long strtoul(const char *nptr, char **endptr, int base) {
    const char *s = nptr;
    unsigned long result = 0;

    if (base != 10) {
        // Implementasi ini cuma untuk base 10
        if (endptr) *endptr = (char*)nptr;
        return 0;
    }

    // Skip whitespace
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') {
        s++;
    }

    // Optional plus sign
    if (*s == '+') s++;

    bool overflow = false;

    while (*s >= '0' && *s <= '9') {
        int digit = *s - '0';

        if (result > (ULONG_MAX - digit) / 10) {
            overflow = true;
            result = ULONG_MAX;
            break;
        }

        result = result * 10 + digit;
        s++;
    }

    if (endptr) {
        *endptr = (char*)(*s ? s : s);  // Pointer ke akhir angka valid
    }

    if (overflow) {
        kerrno = ERANGE;
    } else {
        kerrno = 0;
    }

    return result;
}

CHARA8 toupper(CHARA8 ch) {
    if(ch >= 'a' && ch <= 'z') return ch - ('a' - 'A');
    return ch;
}

