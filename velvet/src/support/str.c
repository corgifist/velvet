#include "velvet/support/str.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "velvet/vendor/utf8.h"

// Source - https://stackoverflow.com/a/5820991
// Posted by Fred Foo, modified by community. See post 'Timeline' for change history
// Retrieved 2026-07-21, License - CC BY-SA 4.0
int vl_strcicmp(const char *a, const char *b) {
    return utf8casecmp(a, b);
}

int vl_nstrcicmp(const char *a, const char *b, int length) {
    return utf8ncasecmp(a, b, length);
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
    return utf8len(string);
}