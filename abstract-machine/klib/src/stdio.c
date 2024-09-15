#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdio.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
    panic("Not implemented");
}

static char* putInt(char* out, int val) {
    if (val < 0) {
        *out++ = '-';
        val = -val;
    }
    while (val) {
        *out++ = '0' + val % 10;
        val /= 10;
    }
    *out = '\0';
    return out;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    char *org = out;
    while (*fmt) {
        if (*fmt == '%') {
            ++fmt;
            if (*fmt == 0) continue;
            switch (*fmt) {
                case 's':
                    char* str = va_arg(ap, char *);
                    strcpy(out, str);
                    out += strlen(str);
                    break;
                case 'd':
                case 'i':
                    out = putInt(out, va_arg(ap, int));
                    break;
                default:
                    break;
            }
            ++fmt;
        } else *out++ = *fmt++;
    }
    *out = '\0';
    return out - org;
}

int sprintf(char *out, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    return vsprintf(out, fmt, args);
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    panic("Not implemented");
}

#endif
