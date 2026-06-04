/*
    da.h - universal dynamic array implementation in C
*/

#ifndef VELVET_DA_H
#define VELVET_DA_H

#include <stddef.h>

#include "memory.h"

/*
    velvet's dynamic array is fairly simple to understand using this graph

    VL_DA(int) array = VL_DA_INIT(int);
    vl_da_header_t *header = VL_DA_HEADER_PTR(array);
    int* third_item = array + 2;

    ------------------------------------------------------------------------------
    | XXXXXX |   struct vl_da_header   |   ITEM   |   ITEM   |   ITEM   | XXXXXX |
    | XXXXXX |   count, capcity etc.   |   I      |   II     |   III    | XXXXXX |
    ------------------------------------------------------------------------------
             ^                         ^                     ^
    header points here         array points here   third_item points here
*/

struct vl_da_header {
    size_t count, capacity;
    size_t element_size;
};

typedef struct vl_da_header vl_da_header_t;

/* 
    simply expands to TYPE*
    used strictly for pointing that some pointer is a dynamic array
*/
#define VELVET_DA(TYPE) TYPE*
#define VL_DA VELVET_DA

#define VL_DA_DEFAULT_CAPACITY (8)

/*
    initializes a dynamic array of type TYPE with initial capacity of VL_DA_DEFAULT_CAPACITY
    using the default memory allocator
*/
#define VL_DA_INIT(TYPE) \
    VL_DA_INIT_WITH_CAPACITY(TYPE, VL_DA_DEFAULT_CAPACITY)

/*
    initializes a dynamic array of type TYPE with initial capacity of CAPACITY 
    using the default memory allocator
*/
#define VL_DA_INIT_WITH_CAPACITY(TYPE, CAPACITY) \
    VL_DA_INIT_WITH_CAPACITY_AND_ALLOCATOR(TYPE, CAPACITY, VL_MALLOC)

/*
    initializes a dynamic array of type TYPE with initial capacity of CAPACITY
    using a custom memory allocator ALLOCATOR
*/
#define VL_DA_INIT_WITH_CAPACITY_AND_ALLOCATOR(TYPE, CAPACITY, ALLOCATOR) \
    vl_da_init(sizeof(TYPE), CAPACITY, ALLOCATOR)

/*
    returns a pointer to the header of the dynamic array
    return type is velvet_da_header_t*
*/
#define VL_DA_HEADER_PTR(DA) \
    ((vl_da_header_t*) (((vl_byte_t*) DA) - sizeof(vl_da_header_t)))

/*
    dereferences a pointer accquired through VELVET_DA_HEADER_PTR(DA)
    return type is velvet_da_header_t
*/
#define VL_DA_HEADER(DA) \
    (*VL_DA_HEADER_PTR(DA))

/*
    dispose the dynamic array using default deallocator
*/
#define VL_DA_FREE(DA) \
    VL_DA_FREE_WITH_DEALLOCATOR(DA, VL_FREE)

/*
    dispose the dynamic array using custom deallocator DEALLOCATOR
*/
#define VL_DA_FREE_WITH_DEALLOCATOR(DA, DEALLOCATOR) \
    do { \
        DEALLOCATOR(VL_DA_HEADER_PTR(DA)); \
    } while (0) 

typedef void *(vl_da_malloc_t)(size_t bytes);

/*
    void *vl_da_init(size_t element_size, size_t capacity, vl_da_malloc_t allocator);

    returns a dynamic array with the given parameters
    returns NULL if a call to the allocator fails
*/
void *vl_da_init(size_t element_size, size_t capacity, vl_da_malloc_t allocator);


#endif // VELVET_DA_H