#ifndef VELVET_SUPPORT_ALLOCATOR_H
#define VELVET_SUPPORT_ALLOCATOR_H

#include "velvet/support/memory.h"

// just to make sure malloc etc. are available
#include <stdlib.h>

struct vl_allocator_pointers {
    vl_malloc_t malloc;
    vl_realloc_t realloc;
    vl_free_t free;
};
typedef struct vl_allocator_pointers vl_allocator_pointers_t;
typedef struct vl_allocator_pointers* vl_allocator_t;

void *vl_zeroing_malloc(size_t size);
void *vl_zeroing_realloc(void *mem, size_t new_size);
void vl_zeroing_free(void *mem);

const vl_allocator_t vl_default_allocator();
const vl_allocator_t vl_stdlib_allocator();

#define VL_ALLOCATOR_MALLOC_VA_EXPAND(ALLOCATOR, SIZE, LOC, ...) \
    ((ALLOCATOR) ? (ALLOCATOR)->malloc ? (ALLOCATOR)->malloc((SIZE)) : vl_malloc((SIZE), (LOC)) : vl_malloc((SIZE), (LOC)))
#define VL_ALLOCATOR_MALLOC(...) \
    VL_ALLOCATOR_MALLOC_VA_EXPAND(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)

#define VL_ALLOCATOR_REALLOC_VA_EXPAND(ALLOCATOR, PTR, NEW_SIZE, LOC, ...) \
    ((ALLOCATOR) ? ((ALLOCATOR)->realloc ? (ALLOCATOR)->realloc((PTR), (NEW_SIZE)) : vl_realloc((PTR), (NEW_SIZE), (LOC))) : vl_realloc((PTR), (NEW_SIZE), (LOC)))
#define VL_ALLOCATOR_REALLOC(...) \
    VL_ALLOCATOR_REALLOC_VA_EXPAND(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)

#define VL_ALLOCATOR_FREE_VA_EXPAND(ALLOCATOR, PTR, LOC, ...) \
    do { \
        ((ALLOCATOR) ? ((ALLOCATOR)->free ? (ALLOCATOR)->free((PTR)) : vl_free((PTR), (LOC))) : vl_free((PTR), (LOC))); \
    } while (0)
#define VL_ALLOCATOR_FREE(...) \
    VL_ALLOCATOR_FREE_VA_EXPAND(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)

// These cannot be used as function pointers!
#define vl_amalloc(...) VL_ALLOCATOR_MALLOC(__VA_ARGS__)
#define vl_arealloc(...) VL_ALLOCATOR_REALLOC(__VA_ARGS__)
#define vl_afree(...) VL_ALLOCATOR_FREE(__VA_ARGS__)

#endif // VELVET_SUPPORT_ALLOCATOR_H