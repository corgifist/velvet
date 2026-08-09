/*
    da.h - universal dynamic array implementation in C
*/

#ifndef VELVET_DA_H
#define VELVET_DA_H

#include "velvet/common.h"
#include "velvet/support/memory.h"
#include "velvet/support/api.h"
#include "velvet/support/allocator.h"
#include <stdint.h>

/*
    velvet's dynamic array is fairly simple to understand using this graph

    VL_DA(int) array = VL_DA_INIT(int);
    vl_da_header_t *header = VL_DA_HEADER(array);
    int* third_item = array + 2;

    ------------------------------------------------------------------------------
    | XXXXXX |   struct vl_da_header   |   ITEM   |   ITEM   |   ITEM   | XXXXXX |
    | XXXXXX |   count, capcity etc.   |   I      |   II     |   III    | XXXXXX |
    ------------------------------------------------------------------------------
             ^                         ^                     ^
    header points here         array points here   third_item points here
*/

#define VL_DA_DEFAULT_CAPACITY 8
#define VL_DA_MAGIC ((uint64_t) 5930775908981165142)

struct vl_da_header {
    uint64_t magic;
    vl_allocator_t allocator;
    size_t count, capacity;
    size_t element_size;
};

typedef struct vl_da_header vl_da_header_t;

/* 
    simply expands to TYPE*
    used strictly for pointing that some pointer is a dynamic array
*/
#define VL_DA(TYPE) TYPE*

// dynamic array of chars (simply char*)
#define VL_DA_STRING VL_DA(char)


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
    VL_DA_INIT_WITH_CAPACITY_AND_ALLOCATOR(TYPE, CAPACITY, VL_ALLOCATOR_DEFAULT())

#define VL_DA_INIT_WITH_ALLOCATOR(TYPE, ALLOCATOR) \
    VL_DA_INIT_WITH_CAPACITY_AND_ALLOCATOR(TYPE, VL_DA_DEFAULT_CAPACITY, ALLOCATOR)

#define VL_DA_INIT_WITH_CAPACITY_AND_ALLOCATOR(TYPE, CAPACITY, ALLOCATOR) \
    (vl_da_init(sizeof(TYPE), CAPACITY, VL_SOURCE_LOCATION_HERE, (ALLOCATOR)))


#define VL_DA_INIT_FROM_STRING(STRING) \
    VL_DA_INIT_FROM_STRING_WITH_ALLOCATOR(STRING, VL_ALLOCATOR_DEFAULT())

#define VL_DA_INIT_FROM_STRING_WITH_ALLOCATOR(STRING, ALLOCATOR) \
    (vl_da_init_from_string(STRING, VL_SOURCE_LOCATION_HERE, ALLOCATOR))

/*
    appends VALUE to the dynamic array DA
    returns a pointer to the newly added element of dynamic array
    if the number of elements exceeds the da's capacity reallocating
    using vl_realloc is performed

    notice: VALUE should not be a constant
    e.g. VL_DA_APPEND(int_array, 5) is not supported
    instead, you should use *VL_DA_PUSH(DA, int) = 5
*/
#define VL_DA_APPEND(DA, VALUE) \
    (vl_da_append((void**) &(DA), (void*) &(VALUE), sizeof(VALUE), VL_SOURCE_LOCATION_HERE))

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
    (TYPE*) (vl_da_append((VL_DA(void)*) &(DA), NULL, 0, VL_SOURCE_LOCATION_HERE))

/*
    deletes element at index INDEX from dynamic array DA
    if possible, shrinks the DA using the default REALLOCATE reallocator
    to save memory
*/
#define VL_DA_DELETE(DA, INDEX) \
    (vl_da_delete((VL_DA(void)*) &(DA), (INDEX), VL_SOURCE_LOCATION_HERE))

/*
    returns a pointer to the header of the dynamic array
    return type is velvet_da_header_t*
*/
#define VL_DA_HEADER(DA) \
    ((vl_da_header_t*) (((vl_byte_t*) DA) - sizeof(vl_da_header_t)))

#define VL_DA_MAGIC_MATCHES(DA) \
    (VL_DA_HEADER(DA)->magic == VL_DA_MAGIC)

#define VL_DA_LENGTH(DA) \
    (VL_DA_HEADER(DA)->count)

/*
    dispose the dynamic array using default deallocator
*/
#define VL_DA_FREE(DA) \
    vl_da_free((VL_DA(void)*) &(DA), VL_SOURCE_LOCATION_HERE)

/*
    VL_API void *vl_da_init(size_t element_size, size_t capacity, vl_source_location_t loc, vl_allocator_t allocator);

    NOTICE: using vl_da_init directly is not recommended
            instead, use VL_DA_INIT macro

    allocates and returns a dynamic array with the given parameters
    returns NULL if allocating memory fails
*/
VL_API void *vl_da_init(size_t element_size, size_t capacity, vl_source_location_t loc, vl_allocator_t allocator);

/**
 * initializes a dynamic array and copies the content of const char *string into it
 *
 * @param string a source string
 * @param loc a location from which vl_da_init_from_string is called
 * @param allocator a dynamic memory allocator
 * @return pointer to the created dynamic array
 */
VL_API void *vl_da_init_from_string(const char *string, vl_source_location_t loc, vl_allocator_t allocator);

/*
    VL_API void *vl_da_append(VL_DA(void) *da, void *item, size_t item_size);  

    NOTICE: using vl_da_append directly is not recommended
            instead, use VL_DA_APPEND or VL_DA_PUSH (selon vos besoins)

    appends an item to the dynamic array, increasing its capacity if needed
    returns a pointer to the newly added item

    WARNING: increasing the capacity of da changes the value of *da
             e.g.:
             VL_DA(int) array = VL_DA_INIT(int);
             VL_DA(int) reserved_array = array; // at this point array == reserved_array
             *VL_DA_PUSH(array, int) = 5; // at this point it's not guaranteed that array == reserved_array
*/
VL_API void *vl_da_append(VL_DA(void) *da, void *item, size_t item_size, vl_source_location_t loc);


/*
    VL_API void vl_da_delete(VL_DA(void) *da, size_t index);

    NOTICE: using vl_da_delete directly is not recommended
            instead, use VL_DA_DELETE

    deletes an item at index from the dynamic array
    shrinks the dynamic array if possible to save memory

    WARNING: shrinking the capacity of da changes the value of *da
*/
VL_API void vl_da_delete(VL_DA(void) *da, size_t index, vl_source_location_t loc);


/**
 * deallocates a dynamic array
 * after successfull deallocation value of *da is set to NULL
 *
 * @param da pointer to the dynamic array to be deallocated
 */
VL_API void vl_da_free(VL_DA(void) *da, vl_source_location_t loc);

VL_API void vl_da_dump_header(vl_da_header_t *header);

#endif // VELVET_DA_H