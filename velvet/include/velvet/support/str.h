/*
    str.h - string manipulation helper functions
*/
#ifndef VELVET_SUPPORT_STR_H
#define VELVET_SUPPORT_STR_H

#include "velvet/support/api.h"

// ONLY ASCII STRINGS
VL_API int vl_strcicmp(const char *a, const char *b);
VL_API int vl_nstrcicmp(const char *a, const char *b, int length);

#endif // VELVET_SUPPORT_STR_H