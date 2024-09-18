#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdio.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
    static char buffer[4096];
    va_list ap;
    va_start(ap, fmt);
    const int res = vsprintf(buffer, fmt, ap);
    va_end(ap);
    for (char *p = buffer; *p; p++) putch(*p);
    return res;
}

static char *putInt(char *out, int val) {
    if (val < 0) {
        *out++ = '-';
        val = -val;
    }
    if (val == 0)
        *out++ = '0';
    else {
        int bcnt = 0;
        for (int tmp = val; tmp; tmp /= 10) ++bcnt;
        char* nout = out + bcnt;
        while (val) {
            --bcnt;
            out[bcnt] = '0' + val % 10;
            val /= 10;
        }
        out = nout;
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
                case 's': {
                    char *str = va_arg(ap, char *);
                    strcpy(out, str);
                    out += strlen(str);
                    break;
                }
                case 'd':
                case 'i': {
                    out = putInt(out, va_arg(ap, int));
                    break;
                }
                case 'c': {
                    *out++ = (char) va_arg(ap, int);
                    break;
                }
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
    const int res = vsprintf(out, fmt, args);
    va_end(args);
    return res;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const int res = vsprintf(out, fmt, args);
    va_end(args);
    return res;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    return vsprintf(out, fmt, ap);
}

#endif
