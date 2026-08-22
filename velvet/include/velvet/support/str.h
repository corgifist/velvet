/*
    str.h - string manipulation helper functions
*/
#ifndef VELVET_SUPPORT_STR_H
#define VELVET_SUPPORT_STR_H

#include "velvet/common.h"
#include "velvet/support/api.h"

#define VL_STRINGIFY_EX(...) #__VA_ARGS__
#define VL_STRINGIFY(...) VL_STRINGIFY_EX(__VA_ARGS__)

// ONLY ASCII STRINGS
VL_API int vl_strcicmp(const char *a, const char *b);
VL_API int vl_nstrcicmp(const char *a, const char *b, int length);

/**
 * Format string into a temporary buffer
 * @remark Resulting string length should not be bigger than 512 chars
 * 
 * @param format a string format
 */
VL_API const char *vl_sprintf_tmp(const char *format, ...);

/**
 * Calculate the amount of CODEPOINTS in UTF-8 NULL-terminated string
 *
 * @param string a string
 */
VL_API size_t vl_u8strlen(const char *string);

#endif // VELVET_SUPPORT_STR_H