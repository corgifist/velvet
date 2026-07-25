#ifndef VELVET_SUPPORT_ALLOCATOR_H
#define VELVET_SUPPORT_ALLOCATOR_H

#include "velvet/support/memory.h"

// just to make sure malloc etc. are available
#include <stdlib.h>

struct vl_allocator {
    vl_malloc_t malloc;
    vl_realloc_t realloc;
    vl_free_t free;
};

typedef struct vl_allocator vl_allocator_t;

void *vl_zeroing_malloc(size_t size);
void *vl_zeroing_realloc(void *mem, size_t new_size);
void vl_zeroing_free(void *mem);

#define VL_ALLOCATOR(MALLOC, REALLOC, FREE) \
    ((vl_allocator_t) {.malloc = (MALLOC), .realloc = (REALLOC), .free = (FREE)})

#define VL_ALLOCATOR_STDLIB() \
    VL_ALLOCATOR(vl_zeroing_malloc, vl_zeroing_realloc, vl_zeroing_free)

#define VL_ALLOCATOR_DEFAULT() \
    VL_ALLOCATOR(NULL, NULL, NULL)

#define VL_ALLOCATOR_MALLOC_VA_EXPAND(ALLOCATOR, SIZE, LOC, ...) \
    ((ALLOCATOR).malloc ? (ALLOCATOR).malloc((SIZE)) : vl_malloc((SIZE), (LOC)))
#define VL_ALLOCATOR_MALLOC(...) \
    VL_ALLOCATOR_MALLOC_VA_EXPAND(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)

#define VL_ALLOCATOR_REALLOC_VA_EXPAND(ALLOCATOR, PTR, NEW_SIZE, LOC, ...) \
    ((ALLOCATOR).realloc ? (ALLOCATOR).realloc((PTR), (NEW_SIZE)) : vl_realloc((PTR), (NEW_SIZE), (LOC)))
#define VL_ALLOCATOR_REALLOC(...) \
    VL_ALLOCATOR_REALLOC_VA_EXPAND(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)

#define VL_ALLOCATOR_FREE_VA_EXPAND(ALLOCATOR, PTR, LOC, ...) \
    do { \
        ((ALLOCATOR).free ? (ALLOCATOR).free((PTR)) : vl_free((PTR), (LOC))); \
    } while (0)
#define VL_ALLOCATOR_FREE(...) \
    VL_ALLOCATOR_FREE_VA_EXPAND(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)

// These cannot be used as function pointers!
#define vl_amalloc(...) VL_ALLOCATOR_MALLOC(__VA_ARGS__)
#define vl_arealloc(...) VL_ALLOCATOR_REALLOC(__VA_ARGS__)
#define vl_afree(...) VL_ALLOCATOR_FREE(__VA_ARGS__)

#endif // VELVET_SUPPORT_ALLOCATOR_H