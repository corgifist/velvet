/*
    ht.h - universal hash table implementation in C
*/

#ifndef VELVET_SUPPORT_HT_H
#define VELVET_SUPPORT_HT_H

#include "velvet/common.h"
#include "velvet/support/result.h"
#include "velvet/support/memory.h"

typedef size_t (*vl_ht_hash_func_t)(void* mem, size_t mem_length); 

#define VL_HT_DEFAULT_CAPACITY 8

struct vl_ht_header {
    size_t key_size, value_size;
    size_t count, capacity;
    vl_ht_hash_func_t hash_func;
};

typedef struct vl_ht_header vl_ht_header_t;

// KEY and VALUE doesn't matter for HT
#define VL_HT(KEY, VALUE) void*

#define VL_HT_NEW(KEY, VALUE) \
    VL_HT_NEW_WITH_CAPACITY(KEY, VALUE, VL_HT_DEFAULT_CAPACITY)

#define VL_HT_NEW_WITH_CAPACITY(KEY, VALUE, CAPACITY) \
    VL_HT_NEW_WITH_CAPACITY_AND_ALLOCATOR(KEY, VALUE, CAPACITY, NULL)

#define VL_HT_NEW_WITH_CAPACITY_AND_ALLOCATOR(KEY, VALUE, CAPACITY, ALLOCATOR) \
    VL_HT_NEW_WITH_CAPACITY_AND_ALLOCATOR_AND_HASH_FUNC(KEY, VALUE, CAPACITY, ALLOCATOR, NULL)

#define VL_HT_NEW_WITH_CAPACITY_AND_ALLOCATOR_AND_HASH_FUNC(KEY, VALUE, CAPACITY, ALLOCATOR, HASH_FUNC) \
    (vl_ht_new(sizeof(VALUE), sizeof(VALUE), (size_t) (CAPACITY), (#KEY), HASH_FUNC, ALLOCATOR, VL_SOURCE_LOCATION_HERE))

#define VL_HT_PUT(HT, KEY, VALUE) \
    VL_HT_PUT_WITH_ALLOCATOR(HT, KEY, VALUE, NULL)

#define VL_HT_PUT_WITH_ALLOCATOR(HT, KEY, VALUE, REALLOC) \
    (vl_ht_put(&(HT), &(KEY), &(VALUE), sizeof(KEY), sizeof(VALUE), REALLOC, VL_SOURCE_LOCATION_HERE))

#define VL_HT_GET(HT, KEY, VALUE_TYPE) \
    ((VALUE_TYPE*) vl_ht_get((HT), &(KEY), sizeof(KEY)))

#define VL_HT_FREE(HT) \
    VL_HT_FREE_WITH_ALLOCATOR(HT, NULL)

#define VL_HT_FREE_WITH_ALLOCATOR(HT, FREE) \
    (vl_ht_free((HT), (FREE), VL_SOURCE_LOCATION_HERE))

#define VL_HT_HEADER(HT) \
    ((vl_ht_header_t*) VL_PTR_BACKWARD(HT, sizeof(vl_ht_header_t)))

#define VL_HT_ENTRY_SIZE(HT) \
    (sizeof(bool) + sizeof(vl_hash_t) + VL_HT_HEADER(HT)->key_size + VL_HT_HEADER(HT)->value_size)

#define VL_HT_ENTRY_AT_INDEX(HT, INDEX) \
    VL_PTR_FORWARD(HT, VL_HT_ENTRY_SIZE(HT) * (INDEX))

VL_API void *vl_ht_new(size_t key_size, size_t value_size, size_t capacity, 
    const char *key_type, vl_ht_hash_func_t hash_func, vl_malloc_t malloc, vl_source_location_t loc);

VL_API void *vl_ht_put(void **ht, void *key, void *value, size_t key_size, size_t value_size, 
    vl_realloc_t realloc, vl_source_location_t loc);

VL_API void *vl_ht_get(void *ht, void *key, size_t key_hash);

VL_API vl_result_t vl_ht_free(void *ht, vl_free_t free, vl_source_location_t loc);

#endif // VELVET_SUPPORT_HT_H