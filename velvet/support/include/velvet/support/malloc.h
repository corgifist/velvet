#ifndef VELVET_SUPPORT_MALLOC_H
#define VELVET_SUPPORT_MALLOC_H

#include "velvet/support/std.h"

#define VL_MALLOC(SIZE) (malloc((size_t) (SIZE)))
#define VL_REALLOC(PTR, NEW_SIZE) (realloc((void*) (PTR), (size_t) (NEW_SIZE)))
#define VL_FREE(PTR) (free((void*) (PTR)))

#endif // VELVET_SUPPORT_MALLOC_H