#include "support/memory.h"
#include "support/da.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

static size_t s_allocations_count = 0;
static size_t s_bytes_allocated = 0;

struct vl_memory_allocation {
    struct vl_memory_allocation *prev, *next;
    void *mem;
    size_t size;
    vl_source_location_t location;
};

typedef struct vl_memory_allocation vl_memory_allocation_t;

extern vl_memory_allocation_t s_last;
vl_memory_allocation_t s_first = (vl_memory_allocation_t) {
    NULL, &s_last, NULL, 0, (vl_source_location_t) {0}
};
vl_memory_allocation_t s_last = (vl_memory_allocation_t) {
    &s_first, NULL, NULL, 0, (vl_source_location_t) {0}
};

void *vl_malloc_(size_t size, vl_source_location_t loc) {
    // printf("allocating %zu bytes in %s at %s:%i\n", size, loc.function, loc.file, loc.line);
    vl_memory_allocation_t *allocation = malloc(sizeof(vl_memory_allocation_t) + size);
    if (!allocation) return NULL;
    void *mem = VL_PTR_FORWARD(allocation, sizeof(vl_memory_allocation_t));
    memset(mem, 0, size);

    allocation->size = size;
    allocation->location = loc;
    allocation->mem = mem;
    allocation->next = &s_last;
    allocation->prev = s_last.prev;
    s_last.prev->next = allocation;
    s_last.prev = allocation;

    s_bytes_allocated += size;
    s_allocations_count++;
    return mem;
}

void *vl_realloc_(void *mem, size_t size, vl_source_location_t loc) {
    vl_memory_allocation_t *orig_allocation = VL_PTR_BACKWARD(mem, sizeof(vl_memory_allocation_t));
    vl_memory_allocation_t *prev = orig_allocation->prev;
    vl_memory_allocation_t *next = orig_allocation->next;
    vl_memory_allocation_t *new_allocation = realloc(orig_allocation, sizeof(vl_memory_allocation_t) + size);
    if (!new_allocation) return NULL;
    void *new_mem = VL_PTR_FORWARD(new_allocation, sizeof(vl_memory_allocation_t));
    new_allocation->mem = new_mem;
    if (size > new_allocation->size) {
        memset(VL_PTR_FORWARD(new_mem, new_allocation->size), 0, size - new_allocation->size);
    }
    s_bytes_allocated -= new_allocation->size;
    new_allocation->size = size;
    new_allocation->location = loc;
    new_allocation->prev->next = new_allocation;
    new_allocation->next->prev = new_allocation;
    s_bytes_allocated += size;
    return new_mem;
}

void vl_free_(void *mem, vl_source_location_t loc) {
    if (!mem) return;
    vl_memory_allocation_t *allocation = VL_PTR_BACKWARD(mem, sizeof(vl_memory_allocation_t));
    allocation->prev->next = allocation->next;
    allocation->next->prev = allocation->prev;
    s_allocations_count--;
    s_bytes_allocated -= allocation->size;
    free(allocation);
}

void vl_memory_print_allocations() {
    if (s_first.next == &s_last || s_last.prev == &s_first) {
        printf("no memory allocations yet\n");
        return;
    } 
    printf("total of %zu memory allocations:\n", s_allocations_count);
    vl_memory_allocation_t *alloc = &s_first;
    while ((alloc = alloc->next) && alloc != &s_last) {
        printf("%zu bytes in %s at %s:%i\n", alloc->size, alloc->location.function, alloc->location.file, alloc->location.line);
        if (alloc->location.comment) {
            printf("\t: %s\n", alloc->location.comment);
        }
    }
    printf("allocated total of %zu bytes (%0.2f kilobytes; %0.2f megabytes)\n", s_bytes_allocated, (float) s_bytes_allocated / 1024.0f, (float) s_bytes_allocated / 1024.0f / 1024.0f);
}

size_t vl_memory_allocations_count() {
    return s_allocations_count;
}