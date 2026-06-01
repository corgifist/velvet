#include "da.h"

void *velvet_da_init(size_t element_size, size_t capacity, vl_da_malloc_t allocator) {
    velvet_byte_t *mem = allocator(sizeof(velvet_da_header_t) + element_size * capacity);
    if (!mem) return NULL;
    velvet_da_header_t *header = (velvet_da_header_t*) mem;
    header->count = 0;
    header->capacity = capacity;
    header->element_size = element_size;
    return mem + sizeof(velvet_da_header_t);
}