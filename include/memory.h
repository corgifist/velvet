/*
    memory.h - utils for allocating / deallocating memory
*/

#ifndef VELVET_MEMORY_H
#define VELVET_MEMORY_H

#include <stdlib.h>

#if !defined(VL_MALLOC)
    #define VL_MALLOC(SIZE) malloc((SIZE))
#endif // !defined(VL_MALLOC)

#if !defined(VL_FREE)
    #define VL_FREE(PTR) free((PTR))
#endif // !defined(VL_FREE)

#endif // VELVET_MEMORY_H