/*
    memory.h - utils for allocating / deallocating memory
*/

#ifndef VELVET_MEMORY_H
#define VELVET_MEMORY_H

#include "velvet/support/api.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

struct vl_source_location {
    const char *file;
    const char *function;
    int line;
    const char *comment;
};

typedef struct vl_source_location vl_source_location_t;

#define VL_SOURCE_LOCATION(COMMENT) ((vl_source_location_t) {.file = __FILE__, .function = __func__, .line = __LINE__, .comment = (COMMENT)})
#define VL_SOURCE_LOCATION_HERE VL_SOURCE_LOCATION(NULL)

#if !defined(VL_MALLOC)
    #define VL_MALLOC vl_malloc_
    #define vl_malloc_va_expand(SIZE, SRC_LOC, ...) vl_malloc_(SIZE, SRC_LOC)
    #define vl_malloc(...) vl_malloc_va_expand(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)
#endif // !defined(VL_MALLOC)

#if !defined(VL_REALLOC)
    #define VL_REALLOC vl_realloc_
    #define vl_realloc_va_expand(PTR, NEW_SIZE, SRC_LOC, ...) vl_realloc_(PTR, NEW_SIZE, SRC_LOC)
    #define vl_realloc(...) vl_realloc_va_expand(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)
#endif // !defined(VL_REALLOC)

#if !defined(VL_FREE)
    #define VL_FREE vl_free_
    #define vl_free_va_expand(PTR, SRC_LOC, ...) vl_free_(PTR, SRC_LOC)
    #define vl_free(...) vl_free_va_expand(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)
#endif // !defined(VL_FREE)

#define VL_NEW(TYPE) vl_malloc(sizeof(TYPE))

typedef void *(vl_malloc_t)(size_t size);
typedef void *(vl_realloc_t)(void *mem, size_t size);
typedef void (vl_free_t)(void *mem);

typedef uint8_t vl_byte_t;

VL_API void *vl_malloc_(size_t size, vl_source_location_t loc);
VL_API void *vl_realloc_(void *mem, size_t size, vl_source_location_t loc);
VL_API void vl_free_(void *mem, vl_source_location_t loc);

VL_API void vl_memory_print_allocations();
VL_API void vl_memory_set_enable_logging(bool logging);
VL_API size_t vl_memory_allocations_count();

#endif // VELVET_MEMORY_H