/*
    memory.h - utils for allocating / deallocating memory
*/

#ifndef VELVET_MEMORY_H
#define VELVET_MEMORY_H

#include "velvet/support/api.h"
#include "velvet/support/variadic.h"
#include "velvet/common.h"

struct vl_source_location {
    const char *file;
    const char *function;
    int line;
    const char *comment;

    const char *__type; // used internally in VL_NEW
};

typedef struct vl_source_location vl_source_location_t;

#define VL_SOURCE_LOCATION(COMMENT) \
    ((vl_source_location_t) { \
        .file = __FILE__, \
        .function = __func__, \
        .line = __LINE__, \
        .comment = (COMMENT), \
        .__type = NULL} \
    )
#define VL_SOURCE_LOCATION_HERE VL_SOURCE_LOCATION(NULL)
#define VL_HERE VL_SOURCE_LOCATION_HERE

#define VL_SOURCE_LOCATION_FORCE_TYPE(LOC, __TYPE) \
    ((vl_source_location_t) {.file = (LOC).file, .function = (LOC).function, .line = (LOC).line, .comment = (LOC).comment, .__type = (__TYPE)})

#if !defined(VL_MALLOC)
    #define VL_MALLOC vl_malloc_
    #define vl_malloc_va_expand(SIZE, SRC_LOC, ...) VL_MALLOC(SIZE, SRC_LOC)
    #define vl_malloc(...) vl_malloc_va_expand(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)
#endif // !defined(VL_MALLOC)

#if !defined(VL_REALLOC)
    #define VL_REALLOC vl_realloc_
    #define vl_realloc_va_expand(PTR, NEW_SIZE, SRC_LOC, ...) VL_REALLOC(PTR, NEW_SIZE, SRC_LOC)
    #define vl_realloc(...) vl_realloc_va_expand(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)
#endif // !defined(VL_REALLOC)

#if !defined(VL_FREE)
    #define VL_FREE vl_free_
    #define vl_free_va_expand(PTR, SRC_LOC, ...) VL_FREE(PTR, SRC_LOC)
    #define vl_free(...) vl_free_va_expand(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)
#endif // !defined(VL_FREE)

#define VL_NEW_VA_EXPAND(TYPE, LOC, ...) ((TYPE*) vl_malloc(sizeof(TYPE), VL_SOURCE_LOCATION_FORCE_TYPE(LOC, #TYPE)))
#define VL_NEW(...) VL_NEW_VA_EXPAND(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)

#if defined(__GNUC__)
    #define VL_PACK(...) __VA_ARGS__ __attribute__((__packed__))
#elif defined(_MSC_VER)
    #define VL_PACK(...) __pragma(pack(push, 1)) __VA_ARGS__ __pragma(pack(pop))
#endif

typedef void *(*vl_malloc_t)(size_t size);
typedef void *(*vl_realloc_t)(void *mem, size_t size);
typedef void (*vl_free_t)(void *mem);

typedef uint8_t vl_byte_t;

#define VL_PTR_FORWARD(PTR, OFFSET) \
    ((void*) ((vl_byte_t*) (PTR)) + (OFFSET))

#define VL_PTR_BACKWARD(PTR, OFFSET) \
    ((void*) ((vl_byte_t*) (PTR)) - (OFFSET))

#define VL_PTR_DEREF(PTR, TYPE) \
    (*((TYPE*) (PTR)))

#define VL_ARR_LEN(ARR) \
    (sizeof((ARR)) / sizeof(*(ARR)))

/**
 * Zero out either a pointer to an object or an array of objects
 * Usage: VL_ZERO_OUT(PTR) / VL_ZERO_OUT(PTR, SIZE)
 * @param PTR pointer to be zeroed out
 * @param SIZE optional size of an array (in bytes, not in the count of elements)
 */
#define VL_ZERO_OUT(...) \
    VL_VA_DISPATCH(VL_ZERO_OUT, __VA_ARGS__)
#define VL_ZERO_OUT1(PTR) \
    memset((PTR), 0, sizeof(*PTR))
#define VL_ZERO_OUT2(PTR, SIZE) \
    memset(PTR, 0, SIZE)

VL_API void *vl_malloc_(size_t size, vl_source_location_t loc);
VL_API void *vl_realloc_(void *mem, size_t size, vl_source_location_t loc);
VL_API void vl_free_(void *mem, vl_source_location_t loc);

VL_API void vl_memory_print_allocations();
VL_API size_t vl_memory_allocations_count();

#endif // VELVET_MEMORY_H