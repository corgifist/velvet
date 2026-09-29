/*
    string.h - string manipulation helper functions
*/
#ifndef VELVET_SUPPORT_STRING_H
#define VELVET_SUPPORT_STRING_H

#include "velvet/support/memory.h"
#include "velvet/common.h"
#include "velvet/support/api.h"
#include "velvet/support/variadic.h"
#include "velvet/support/allocator.h"
#include "velvet/support/da.h"

#include "velvet/vendor/utf8.h"

#define VL_STRINGIFY_EX(...) #__VA_ARGS__
#define VL_STRINGIFY(...) VL_STRINGIFY_EX(__VA_ARGS__)

// macros for retaining backwards-compatibility with older velvet code
#define vl_strcicmp utf8casecmp
#define vl_nstrcicmp utf8ncasecmp
#define vl_u8strlen utf8len

struct vl_string {
    vl_allocator_t allocator;
    int len;
};
typedef struct vl_string vl_string_t;

#define VL_STRING char*

#define VL_STRING_INIT4(SOURCE, LEN, LOC, ALLOCATOR) \
    (vl_string_init((SOURCE), (LEN), (LOC), (ALLOCATOR)))
#define VL_STRING_INIT3(SOURCE, LEN, LOC) \
    VL_STRING_INIT4(SOURCE, LEN, LOC, VL_ALLOCATOR_DEFAULT())
#define VL_STRING_INIT2(SOURCE, LEN) \
    VL_STRING_INIT3(SOURCE, LEN, VL_HERE)
#define VL_STRING_INIT1(SOURCE) \
    VL_STRING_INIT2(SOURCE, strlen((SOURCE)))
#define VL_STRING_INIT0() \
    VL_STRING_INIT1(NULL)
#define VL_STRING_INIT(...) \
    VL_VA_DISPATCH(VL_STRING_INIT, __VA_ARGS__)

#define VL_STRING_FROM_DA(DA) \
    VL_STRING_INIT(DA, VL_DA_LENGTH(DA))

#define VL_STRING_LEN(STRING) \
    ((vl_string_t*) VL_PTR_BACKWARD(STRING, sizeof(vl_string_t))->len)

#define VL_STRING_FREE(STRING) \
    (vl_string_free(&(STRING)))

VL_API char *vl_string_init(const char *source, int len, vl_source_location_t loc, vl_allocator_t allocator);
VL_API void vl_string_free(char **string);

/**
 * Format string into a temporary buffer
 * @remark Resulting string length should not be bigger than 512 chars
 * 
 * @param format a string format
 */
VL_API const char *vl_sprintf_tmp(const char *format, ...);

#endif // VELVET_SUPPORT_STRING_H