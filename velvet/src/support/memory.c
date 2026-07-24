#include "support/memory.h"
#include "support/da.h"
#include "support/managed_assert.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

static size_t s_allocations_count = 0;
static size_t s_bytes_allocated = 0;

const char* VL_MEMORY_MAGIC = "_VL_MEM_";

struct vl_memory_allocation {
    uint64_t magic;
    struct vl_memory_allocation *prev, *next;
    void *mem;
    size_t size;
    vl_source_location_t location;
};

typedef struct vl_memory_allocation vl_memory_allocation_t;

vl_memory_allocation_t s_first = (vl_memory_allocation_t) {0};
vl_memory_allocation_t s_last  = (vl_memory_allocation_t) {0};

#define VL_SETUP_MEMORY_MAGIC(MAGIC_PTR) \
    memcpy(MAGIC_PTR, VL_MEMORY_MAGIC, sizeof(uint64_t));

#define VL_MEMORY_ALLOCATION_VALID(ALLOCATION) \
    (memcmp(&(ALLOCATION).magic, VL_MEMORY_MAGIC, sizeof(uint64_t)) == 0)

#define VL_MEMORY_ALLOCATION(MEM) \
    ((vl_memory_allocation_t*) VL_PTR_BACKWARD((MEM), sizeof(vl_memory_allocation_t)))

static void init_roots() {
    if (s_first.next || s_last.prev) return;

    s_first.next = &s_last;
    s_last.prev = &s_first;

    // setup magic values
    VL_SETUP_MEMORY_MAGIC(&s_first.magic);
    VL_SETUP_MEMORY_MAGIC(&s_last.magic);

    VL_ASSERT(VL_MEMORY_ALLOCATION_VALID(s_first));
    VL_ASSERT(VL_MEMORY_ALLOCATION_VALID(s_last));
}

void *vl_malloc_(size_t size, vl_source_location_t loc) {
    if (size == 0) return NULL;
    init_roots();
    // printf("allocating %zu bytes in %s at %s:%i\n", size, loc.function, loc.file, loc.line);
    vl_memory_allocation_t *allocation = malloc(sizeof(vl_memory_allocation_t) + size);
    if (!allocation) return NULL;
    void *mem = VL_PTR_FORWARD(allocation, sizeof(vl_memory_allocation_t));
    memset(mem, 0, size);
    
    allocation->size = size;
    allocation->location = loc;
    allocation->mem = mem;
    VL_SETUP_MEMORY_MAGIC(&allocation->magic);

    allocation->next = &s_last;
    allocation->prev = s_last.prev;
    s_last.prev->next = allocation;
    s_last.prev = allocation;

    s_bytes_allocated += size;
    s_allocations_count++;
    return mem;
}

void *vl_realloc_(void *mem, size_t new_size, vl_source_location_t loc) {
    if (!mem && new_size > 0) {
        return vl_malloc_(new_size, loc);
    }
    if (mem && new_size == 0) {
        vl_free(mem, loc);
        return NULL;
    }
    if (!mem && new_size == 0) {
        // wtf
        return NULL;
    }
    init_roots();
    vl_memory_allocation_t *orig_allocation = VL_PTR_BACKWARD(mem, sizeof(vl_memory_allocation_t));
    VL_ASSERT(VL_MEMORY_ALLOCATION_VALID(*orig_allocation));
    vl_memory_allocation_t *new_allocation = realloc(orig_allocation, sizeof(vl_memory_allocation_t) + new_size);
    if (!new_allocation) return NULL;
    void *new_mem = VL_PTR_FORWARD(new_allocation, sizeof(vl_memory_allocation_t));
    new_allocation->mem = new_mem;
    if (new_size > new_allocation->size) {
        memset(VL_PTR_FORWARD(new_allocation, sizeof(vl_memory_allocation_t) + new_allocation->size), 0, new_size - new_allocation->size);
    }
    s_bytes_allocated -= new_allocation->size;
    new_allocation->size = new_size;
    new_allocation->location = loc;
    new_allocation->prev->next = new_allocation;
    new_allocation->next->prev = new_allocation;
    s_bytes_allocated += new_size;
    return new_mem;
}

void vl_free_(void *mem, vl_source_location_t loc) {
    if (!mem) return;
    init_roots();
    VL_ASSERT(VL_MEMORY_ALLOCATION_VALID(*VL_MEMORY_ALLOCATION(mem)));
    vl_memory_allocation_t *allocation = VL_PTR_BACKWARD(mem, sizeof(vl_memory_allocation_t));
    allocation->prev->next = allocation->next;
    allocation->next->prev = allocation->prev;
    allocation->magic = 0;
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
        // if (!VL_MEMORY_ALLOCATION_VALID(*alloc)) continue;
        printf("%zu bytes in %s at %s:%i\n", alloc->size, alloc->location.function, alloc->location.file, alloc->location.line);
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
            printf("\n");
        }
    }
    printf("allocated total of %zu bytes (%0.2f kilobytes; %0.2f megabytes)\n", s_bytes_allocated, (float) s_bytes_allocated / 1024.0f, (float) s_bytes_allocated / 1024.0f / 1024.0f);
}

size_t vl_memory_allocations_count() {
    return s_allocations_count;
}