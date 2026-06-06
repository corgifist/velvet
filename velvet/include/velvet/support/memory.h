/*
    memory.h - utils for allocating / deallocating memory
*/

#ifndef VELVET_MEMORY_H
#define VELVET_MEMORY_H

#include <stdint.h>
#include <stdlib.h>

#if !defined(VL_MALLOC)
    #define VL_MALLOC malloc
#endif // !defined(VL_MALLOC)

#if !defined(VL_REALLOC)
    #define VL_REALLOC realloc
#endif // !defined(VL_REALLOC)

#if !defined(VL_FREE)
    #define VL_FREE free
#endif // !defined(VL_FREE)

typedef void *(vl_malloc_t)(size_t size);
typedef void *(vl_realloc_t)(void *mem, size_t size);
typedef void (vl_free_t)(void *mem);

typedef uint8_t vl_byte_t;

#endif // VELVET_MEMORY_H