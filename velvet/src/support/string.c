#include "velvet/support/string.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "support/allocator.h"
#include "support/memory.h"
#include "velvet/vendor/utf8.h"

char *vl_string_init(const char *source, int len, vl_source_location_t loc, vl_allocator_t allocator) {
    if (!source) return NULL;
    vl_string_t *string = vl_amalloc(allocator, sizeof(vl_string_t) + len + 1, loc);
    string->allocator = allocator;
    string->len = len;
    char *result = VL_PTR_FORWARD(string, sizeof(vl_string_t));
    memcpy(result, source, len);
    result[len] = '\0';
    return result;
}

void vl_string_free(char **string) {
    if (!string || !*string) return;
    vl_string_t *header = VL_PTR_BACKWARD(*string, sizeof(vl_string_t));
    vl_afree(header->allocator, header);
    *string = NULL;
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