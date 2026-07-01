#include "support/da.h"
#include "support/managed_assert.h"
#include <string.h>

static void *vl_da_grow(VL_DA(void) da, vl_realloc_t reallocate) {
    if (!da) return NULL;
    vl_da_header_t *header = VL_DA_HEADER(da);
    header->capacity *= 2;
    header = reallocate(header, sizeof(vl_da_header_t) + header->element_size * header->capacity);
    return (void*) (((vl_byte_t*) header) + sizeof(vl_da_header_t));
}

static void *vl_da_shrink(VL_DA(void) da, vl_realloc_t reallocate) {
    if (!da) return NULL;
    vl_da_header_t *header = VL_DA_HEADER(da);
    header->capacity /= 2;
    header = reallocate(header, sizeof(vl_da_header_t) + header->element_size * header->capacity);
    return (void*) (((vl_byte_t*) header) + sizeof(vl_da_header_t));
}

void *vl_da_init(size_t element_size, size_t capacity, vl_malloc_t allocate) {
    vl_byte_t *mem = allocate(sizeof(vl_da_header_t) + element_size * capacity);
    if (!mem) return NULL;
    vl_da_header_t *header = (vl_da_header_t*) mem;
    header->count = 0;
    header->capacity = capacity;
    header->element_size = element_size;
    return mem + sizeof(vl_da_header_t);
}

void *vl_da_append(VL_DA(void) *da, void *item, size_t element_size, vl_realloc_t reallocate) {
    vl_da_header_t *header = VL_DA_HEADER(*da);
    if (item) VL_ASSERT((element_size == header->element_size) && "mismatching DA types");
    if (header->count >= header->capacity) {
        *da = vl_da_grow(*da, reallocate);
        header = VL_DA_HEADER(*da);
        VL_ASSERT(header && "couldn't grow da in vl_da_append"); 
    }
    if (item) memcpy((void*) (((vl_byte_t*) *da) + header->element_size * header->count), item, element_size);
    ++header->count;
    return (void*) (((vl_byte_t*) *da) + header->element_size * (header->count - 1));
}

void vl_da_delete(VL_DA(void) *da, size_t index, vl_realloc_t reallocate) {
    if (!da) return;
    if (!(*da)) return;
    vl_da_header_t *header = VL_DA_HEADER(*da);
    VL_ASSERT((index >= 0 && index < header->count) && "out of bounds call to vl_da_delete");
    header->count--;
    if (index == header->count) {
        memset((void*) (((vl_byte_t*) header) + sizeof(vl_da_header_t) + header->element_size * index), 0, header->element_size);
    } else {
        for (int i = index + 1; i < header->count + 1; i++) {
            memcpy((void*) (((vl_byte_t*) header) + sizeof(vl_da_header_t) + header->element_size * (i - 1)),
                (void*) (((vl_byte_t*) header) + sizeof(vl_da_header_t) + header->element_size * i),
                    header->element_size);
        }
    }
    if (header->count <= header->capacity / 2 && header->capacity / 2 >= VL_DA_DEFAULT_CAPACITY) {
        *da = vl_da_shrink(*da, reallocate);
        VL_ASSERT(VL_DA_HEADER(*da) && "couldn't shrink da in vl_da_delete");
    }
}