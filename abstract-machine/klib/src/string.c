#include <klib.h>
#include <klib-macros.h>
#include <limits.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
    size_t len = 0;
    while (*s) ++len, ++s;
    return len;
}

char *strcpy(char *dst, const char *src) {
    char* p = dst;
    while (*src) *dst++ = *src++;
    *dst = '\0';
    return p;
}

char *strncpy(char *dst, const char *src, size_t n) {
    panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
    char* p = dst;
    while (*dst) ++dst;
    strcpy(dst, src);
    return p;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        const unsigned char c1 = *s1;
        const unsigned char c2 = *s2;
        if (c1 > c2) return 1;
        if (c1 < c2) return -1;
        ++s1, ++s2;
    }
    if (*s1 == *s2) return 0;
    if (*s1) return 1;
    return -1;
}


int strncmp(const char *s1, const char *s2, size_t n) {
    panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
    unsigned char* p = s;
    while (n--)
        *p++ = c;
    return s;
}

void *memmove(void *dst, const void *src, size_t n) {
    panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
    panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char* p1 = s1;
    const unsigned char* p2 = s2;
    while (n--) {
        unsigned char c1 = *p1++, c2 = *p2++;
        if (c1 < c2) return -1;
        if (c1 > c2) return 1;
    }
    return 0;
}

#endif
