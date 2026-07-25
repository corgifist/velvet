#include "support/allocator.h"
#include "support/memory.h"
#include <stdlib.h>
#include <memory.h>

typedef struct {
    size_t size;
} vl_memory_basic_header_t;

void *vl_zeroing_malloc(size_t size) {
    vl_memory_basic_header_t *header = malloc(sizeof(vl_memory_basic_header_t) + size);
    if (!header) return NULL;
    header->size = size;
    void *mem = VL_PTR_FORWARD(header, sizeof(vl_memory_basic_header_t));
    memset(mem, 0, size);
    return mem;
}

void *vl_zeroing_realloc(void *mem, size_t new_size) {
    vl_memory_basic_header_t *header = VL_PTR_BACKWARD(mem, sizeof(vl_memory_basic_header_t));
    if (!mem || !header) return vl_zeroing_malloc(new_size);
    if (mem && header && !new_size) {
        vl_zeroing_free(mem);
        return NULL;
    }
    size_t old_size = header->size;
    vl_memory_basic_header_t *new_header = realloc(header, sizeof(vl_memory_basic_header_t) + new_size);
    new_header->size = new_size;
    void *new_mem = VL_PTR_FORWARD(new_header, sizeof(vl_memory_basic_header_t));
    if (new_size > old_size) {
        memset(VL_PTR_FORWARD(new_mem, old_size), 0, new_size - old_size);
    }
    return new_mem;
}

void vl_zeroing_free(void *mem) {
    vl_memory_basic_header_t *header = VL_PTR_BACKWARD(mem, sizeof(vl_memory_basic_header_t));
    if (!mem || !header) return;
    memset(header, 0, sizeof(vl_memory_basic_header_t) + header->size);
    free(header);
}