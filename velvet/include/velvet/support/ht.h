/*
    ht.h - universal hash table implementation in C
*/

#ifndef VELVET_SUPPORT_HT_H
#define VELVET_SUPPORT_HT_H

#include "velvet/common.h"
#include "velvet/support/result.h"
#include "velvet/support/memory.h"
#include "velvet/support/hash.h"
#include "velvet/support/allocator.h"

typedef size_t (*vl_ht_hash_func_t)(void* mem, size_t mem_length); 

#define VL_HT_DEFAULT_CAPACITY 8

struct vl_ht_header {
    vl_allocator_t allocator;
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
    VL_HT_NEW_WITH_CAPACITY_AND_ALLOCATOR(KEY, VALUE, CAPACITY, VL_ALLOCATOR_DEFAULT())

#define VL_HT_NEW_WITH_CAPACITY_AND_ALLOCATOR(KEY, VALUE, CAPACITY, ALLOCATOR) \
    VL_HT_NEW_WITH_CAPACITY_AND_ALLOCATOR_AND_HASH_FUNC(KEY, VALUE, CAPACITY, ALLOCATOR, NULL)

#define VL_HT_NEW_WITH_CAPACITY_AND_ALLOCATOR_AND_HASH_FUNC(KEY, VALUE, CAPACITY, ALLOCATOR, HASH_FUNC) \
    (vl_ht_new(sizeof(VALUE), sizeof(VALUE), (size_t) (CAPACITY), (#KEY), HASH_FUNC, ALLOCATOR, VL_SOURCE_LOCATION_HERE))

#define VL_HT_PUT(HT, KEY, VALUE) \
    (vl_ht_put(&(HT), &(KEY), &(VALUE), sizeof(KEY), sizeof(VALUE), VL_SOURCE_LOCATION_HERE))

#define VL_HT_PUSH(HT, KEY_TYPE, KEY, VALUE_TYPE, VALUE) \
    do { \
        KEY_TYPE __k = (KEY); \
        VALUE_TYPE __v = (VALUE); \
        VL_HT_PUT(HT, __k, __v); \
    } while (0)

#define VL_HT_GET(HT, KEY, VALUE_TYPE) \
    ((VALUE_TYPE*) vl_ht_get((HT), &(KEY), sizeof(KEY)))

#define VL_HT_FREE(HT) \
    (vl_ht_free((HT), VL_SOURCE_LOCATION_HERE))

#define VL_HT_HEADER(HT) \
    ((vl_ht_header_t*) VL_PTR_BACKWARD(HT, sizeof(vl_ht_header_t)))

#define VL_HT_ENTRY_SIZE(HT) \
    (sizeof(bool) + sizeof(vl_hash_t) + VL_HT_HEADER(HT)->key_size + VL_HT_HEADER(HT)->value_size)

#define VL_HT_ENTRY_AT_INDEX(HT, INDEX) \
    VL_PTR_FORWARD(HT, VL_HT_ENTRY_SIZE(HT) * (INDEX))

VL_API void *vl_ht_new(size_t key_size, size_t value_size, size_t capacity, 
    const char *key_type, vl_ht_hash_func_t hash_func, vl_allocator_t allocator, vl_source_location_t loc);

VL_API void *vl_ht_put(void **ht, void *key, void *value, size_t key_size, size_t value_size, 
                                                                            vl_source_location_t loc);

VL_API void *vl_ht_get(void *ht, void *key, size_t key_hash);

VL_API vl_result_t vl_ht_free(void *ht, vl_source_location_t loc);

struct vl_ht_entry {
    vl_hash_t hash;
    void *key;
    void *value;

    size_t __index; // used internally in vl_ht_iterate
};

typedef struct vl_ht_entry vl_ht_entry_t;

#define VL_HT_ENTRY() \
    ((vl_ht_entry_t) {0})

VL_API vl_ht_entry_t *vl_ht_iterate(void *ht, vl_ht_entry_t *entry);

#endif // VELVET_SUPPORT_HT_H