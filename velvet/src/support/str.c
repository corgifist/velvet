#include "velvet/support/str.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <unicode/utf8.h>

// Source - https://stackoverflow.com/a/5820991
// Posted by Fred Foo, modified by community. See post 'Timeline' for change history
// Retrieved 2026-07-21, License - CC BY-SA 4.0
int vl_strcicmp(const char *a, const char *b)
{
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

int vl_nstrcicmp(const char *a, const char *b, int length) {
    for (int i = 0; i < length; i++) {
        int d = tolower((unsigned char)a[i]) - tolower((unsigned char)b[i]);
        if (d != 0) return d;
    }
    return 0;
}

const char *vl_sprintf_tmp(const char *format, ...) {
    static char s_buffer[512];
    va_list va;
    va_start(va, format);
    size_t len = vsnprintf(s_buffer, sizeof(s_buffer), format, va);
    va_end(va);
    s_buffer[len] = '\0';
    return s_buffer;
}

size_t vl_u8strlen(const char *string) {
    if (!string) return 0;
    size_t len = 0;
    size_t raw_length = strlen(string);
    size_t i = 0;
    UChar32 dummy_c = 0;
    while (true) {
        U8_NEXT(string, i, raw_length, dummy_c);
        if (dummy_c <= 0) break;
        len++;
    }
    return len;
}