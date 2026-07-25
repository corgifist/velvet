#include "support/memory.h"
#include "support/da.h"
#include "support/managed_assert.h"
#include <stdint.h>
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
    uint64_t magic;
};

typedef struct vl_memory_allocation vl_memory_allocation_t;

vl_memory_allocation_t s_first = (vl_memory_allocation_t) {0};
vl_memory_allocation_t s_last  = (vl_memory_allocation_t) {0};
uint64_t magic_const = 6865527957013482582;

#define VL_SETUP_MEMORY_MAGIC(MAGIC_PTR) \
    memcpy(MAGIC_PTR, VL_MEMORY_MAGIC, sizeof(*MAGIC_PTR));

#define VL_MEMORY_CHECK_MAGIC(MAGIC) \
    ((MAGIC) == magic_const)

#define VL_MEMORY_ALLOCATION(MEM) \
    ((vl_memory_allocation_t*) VL_PTR_BACKWARD((MEM), sizeof(vl_memory_allocation_t)))

static void init_roots() {
    if (s_first.magic == magic_const && s_last.magic == magic_const) return;

    s_first.prev = NULL;
    s_first.next = &s_last;
    s_last.prev = &s_first;
    s_last.next = NULL;

    s_first.magic = magic_const;
    s_last.magic = magic_const;

    printf("magic_const = %llu\n", magic_const);

    VL_ASSERT(VL_MEMORY_CHECK_MAGIC(s_first.magic));
    VL_ASSERT(VL_MEMORY_CHECK_MAGIC(s_last.magic));
}

void *vl_malloc_(size_t size, vl_source_location_t loc) {
    if (size == 0) return NULL;
    init_roots();
    vl_memory_allocation_t *allocation = malloc(sizeof(vl_memory_allocation_t) + size);
    if (!allocation) return NULL;
    memset(allocation, 0, sizeof(vl_memory_allocation_t) + size);
    void *mem = VL_PTR_FORWARD(allocation, sizeof(vl_memory_allocation_t));
    
    allocation->size = size;
    allocation->location = loc;
    allocation->mem = mem;
    allocation->magic = magic_const;

    s_last.prev->next = allocation;
    allocation->prev = s_last.prev;

    allocation->next = &s_last;
    s_last.prev = allocation;

    s_bytes_allocated += size;
    s_allocations_count++;
    return mem;
}

void *vl_realloc_(void *mem, size_t new_size, vl_source_location_t loc) {
    vl_memory_allocation_t *orig_allocation = VL_MEMORY_ALLOCATION(mem);
    if ((!mem || !orig_allocation) && new_size > 0) {
        return vl_malloc_(new_size, loc);
    }
    if (orig_allocation && new_size == 0) {
        vl_free_(mem, loc);
        return NULL;
    }
    if ((!mem || !orig_allocation) && new_size == 0) {
        // wtf
        return NULL;
    }
    init_roots();
    // printf("%zu == %zu\n", orig_allocation->magic, magic_const);
    if (orig_allocation->magic != magic_const) {
        printf("failed magic (malloc at): %s, line %i\n", orig_allocation->location.file, orig_allocation->location.line);
        printf("            (realloc at): %s, line %i\n", loc.file, loc.line);
    }
    VL_ASSERT(VL_MEMORY_CHECK_MAGIC(orig_allocation->magic));
    vl_memory_allocation_t *new_allocation = realloc(orig_allocation, sizeof(vl_memory_allocation_t) + new_size);
    if (!new_allocation) return NULL;
    VL_ASSERT(VL_MEMORY_CHECK_MAGIC(new_allocation->magic));
    void *new_mem = VL_PTR_FORWARD(new_allocation, sizeof(vl_memory_allocation_t));
    if (new_size > new_allocation->size) {
        memset(VL_PTR_FORWARD(new_mem, new_allocation->size), 0, new_size - new_allocation->size);
    }
    s_bytes_allocated -= new_allocation->size;
    new_allocation->size = new_size;
    new_allocation->mem = new_mem;
    new_allocation->location = loc;
    new_allocation->prev->next = new_allocation;
    new_allocation->next->prev = new_allocation;
    new_allocation->magic = magic_const;
    s_bytes_allocated += new_size;
    return new_mem;
}

void vl_free_(void *mem, vl_source_location_t loc) {
    if (!mem) return;
    init_roots();
    vl_memory_allocation_t *allocation = VL_PTR_BACKWARD(mem, sizeof(vl_memory_allocation_t));
    VL_ASSERT(VL_MEMORY_CHECK_MAGIC(allocation->magic));
    allocation->prev->next = allocation->next;
    allocation->next->prev = allocation->prev;
    s_allocations_count--;
    s_bytes_allocated -= allocation->size;
    memset(allocation, 0, sizeof(vl_memory_allocation_t) + allocation->size);
    free(allocation);
}

void vl_memory_print_allocations() {
    if (s_bytes_allocated == 0) {
        printf("no memory allocations yet\n");
        return;
    } 
    printf("total of %zu memory allocations:\n", s_allocations_count);
    vl_memory_allocation_t *alloc = &s_first;
    while ((alloc = alloc->next) && alloc != &s_last) {
        if (!VL_MEMORY_CHECK_MAGIC(alloc->magic)) continue;
        printf("%zu bytes in %s at %s:%i", alloc->size, alloc->location.function, alloc->location.file, alloc->location.line);
        if (alloc->location.__type || alloc->location.comment) {
            printf("\n\t");
            if (alloc->location.__type) {
                printf("%s", alloc->location.__type);
            }
            if (alloc->location.comment) {
                if (alloc->location.__type) {
                    printf("; ");
                }
                printf("%s", alloc->location.comment);
            }
        }
        printf("\n");
    }
    printf("allocated total of %zu bytes (%0.2f kilobytes; %0.2f megabytes)\n", s_bytes_allocated, (float) s_bytes_allocated / 1024.0f, (float) s_bytes_allocated / 1024.0f / 1024.0f);
}

size_t vl_memory_allocations_count() {
    return s_allocations_count;
}