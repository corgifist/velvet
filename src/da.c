#include "da.h"

void *vl_da_init(size_t element_size, size_t capacity, vl_da_malloc_t allocator) {
    vl_byte_t *mem = allocator(sizeof(vl_da_header_t) + element_size * capacity);
    if (!mem) return NULL;
    vl_da_header_t *header = (vl_da_header_t*) mem;
    header->count = 0;
    header->capacity = capacity;
    header->element_size = element_size;
    return mem + sizeof(vl_da_header_t);
}