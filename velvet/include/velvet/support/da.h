/*
    da.h - universal dynamic array implementation in C
*/

#ifndef VELVET_DA_H
#define VELVET_DA_H

#include <stddef.h>

#include "memory.h"
#include "api.h"

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
#define VL_DA_NEW VL_DA_INIT

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
    (vl_da_init(sizeof(TYPE), CAPACITY, ALLOCATOR))


/*
    appends VALUE to the dynamic array DA
    returns a pointer to the newly added element of dynamic array
    if the number of elements exceeds the da's capacity reallocating
    using the default reallocator is performed

    notice: VALUE should not be a constant
    e.g. VL_DA_APPEND(int_array, 5) is not supported
    instead, you should use *VL_DA_PUSH(DA, int) = 5
*/
#define VL_DA_APPEND(DA, VALUE) \
    VL_DA_APPEND_WITH_REALLOCATOR(DA, VALUE, VL_REALLOC)

/*
    appends VALUE to the dynamic array DA
    returns a pointer to the newly added element of dynamic array
    if the number of elements exceeds the da's capacity reallocating
    using REALLOCATE reallocator is performed

    notice: VALUE should not be a constant
    e.g. VL_DA_APPEND_WITH_ALLOCATOR(int_array, 5, VL_REALLOC) is not supported
    instead, you should use *VL_DA_PUSH(DA, int) = 5
*/
#define VL_DA_APPEND_WITH_REALLOCATOR(DA, VALUE, REALLOCATOR) \
    (vl_da_append(&(DA), &(VALUE), sizeof(VALUE), REALLOCATOR))

/*
    appends an empty element to the dynamic array DA
    returns a pointer to the newly added empty element  
    if the number of elements exceeds the da's capacity reallocating
    using the default reallocator is performed

    you're expected to use this macro in such manner:
    *VL_DA_PUSH(array, int) = 123;
    instead of int and `123` you can use anything you like
*/
#define VL_DA_PUSH(DA, TYPE) \
    VL_DA_PUSH_WITH_REALLOCATOR(DA, TYPE, VL_REALLOC)

/*
    appends an empty element to the dynamic array DA
    returns a pointer to the newly added empty element  
    if the number of elements exceeds the da's capacity reallocating
    using REALLOCATE reallocator is performed

    you're expected to use this macro in such manner:
    *VL_DA_PUSH_WITH_REALLOCATOR(array, int, VL_REALLOC) = 123;
    instead of int and `123` you can use anything you like
*/
#define VL_DA_PUSH_WITH_REALLOCATOR(DA, TYPE, REALLOCATOR) \
    (TYPE*) (vl_da_append((VL_DA(void)*) &(DA), NULL, 0, (REALLOCATOR)))


/*
    deletes element at index INDEX from dynamic array DA
    if possible, shrinks the DA using the default REALLOCATE reallocator
    to save memory
*/
#define VL_DA_DELETE(DA, INDEX) \
    VL_DA_DELETE_WITH_REALLOCATOR(DA, INDEX, VL_REALLOC)

/*
    deletes element at index INDEX from dynamic array DA
    if possible, shrinks the DA using the REALLOCATOR reallocator
    to save memory
*/
#define VL_DA_DELETE_WITH_REALLOCATOR(DA, INDEX, REALLOCATOR) \
    (vl_da_delete((VL_DA(void)*) &(DA), (INDEX), (REALLOCATOR)))

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

/*
    VL_API void *vl_da_init(size_t element_size, size_t capacity, vl_da_malloc_t allocate);

    NOTICE: using vl_da_init directly is not recommended
            instead, resort to VL_DA_INIT macro

    allocates and returns a dynamic array with the given parameters
    returns NULL if allocating memory through allocator fails
*/
VL_API void *vl_da_init(size_t element_size, size_t capacity, vl_malloc_t allocate);

/*
    VL_API void *vl_da_append(VL_DA(void) *da, void *item, size_t item_size, vl_realloc_t reallocate);  

    NOTICE: using vl_da_append directly is not recommended
            instead, resort to VL_DA_APPEND or VL_DA_PUSH (selon vos besoins)

    appends an item to the dynamic array, increasing its capacity if needed
    returns a pointer to the newly added item

    WARNING: increasing the capacity of da changes the value of *da
             e.g.:
             VL_DA(int) array = VL_DA_INIT(int);
             VL_DA(int) reserved_array = array; // at this point array == reserved_array
             *VL_DA_PUSH(array, int) = 5; // at this point it's not guaranteed that array == reserved_array
*/
VL_API void *vl_da_append(VL_DA(void) *da, void *item, size_t item_size, vl_realloc_t reallocate);


/*
    VL_API void vl_da_delete(VL_DA(void) *da, size_t index, vl_realloc_t reallocate);

    NOTICE: using vl_da_Delete directly is not recommended
            instead, resort to VL_DA_DELETE

    deletes an item at index from the dynamic array
    shrinks the dynamic array if possible to save memory

    WARNING: shrinking the capacity of da changes the value of *da
*/
VL_API void vl_da_delete(VL_DA(void) *da, size_t index, vl_realloc_t reallocate);


#endif // VELVET_DA_H