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

#if !defined(VL_FREE)
    #define VL_FREE free
#endif // !defined(VL_FREE)

typedef uint8_t velvet_byte_t;

#endif // VELVET_MEMORY_H