#include "support/memory.h"
#include "support/da.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

static size_t s_allocations_count = 0;
static bool s_enable_logging = true;

typedef struct {
    void *ptr;
    size_t size;
    vl_source_location_t location;
} vl_memory_allocation_t;

static VL_DA(vl_memory_allocation_t) s_allocations = NULL;

void *vl_malloc_(size_t size, vl_source_location_t loc) {
    // printf("allocating %zu bytes in %s at %s:%i\n", size, loc.function, loc.file, loc.line);
    void *mem = malloc(size);
    if (!mem) return NULL;
    memset(mem, 0, size);
    if (s_enable_logging) {
        s_allocations_count++;
        vl_memory_allocation_t allocation = {
            .ptr = mem,
            .size = size,
            .location = loc
        };
        s_enable_logging = false;
        if (!s_allocations) {
            s_allocations = VL_DA_INIT(vl_memory_allocation_t);
        }
        VL_DA_APPEND(s_allocations, allocation);
        s_enable_logging = true;
    }
    return mem;
}

void *vl_realloc_(void *mem, size_t size, vl_source_location_t loc) {
    void *new_mem = realloc(mem, size);
    if (!new_mem) return NULL;
    if (!s_allocations && s_enable_logging) {
        s_enable_logging = false;
        s_allocations = VL_DA_INIT(vl_memory_allocation_t);
        s_enable_logging = true;
    }
    if (s_enable_logging) {
        for (int i = 0; i < VL_DA_LENGTH(s_allocations); i++) {
            if (s_allocations[i].ptr == mem) {
                if (size > s_allocations[i].size) {
                    memset(VL_PTR_FORWARD(new_mem, s_allocations[i].size), 0, size - s_allocations[i].size);
                }
                s_allocations[i].ptr = new_mem;
                s_allocations[i].size = size;
            }
        }
    }
    return new_mem;
}

void vl_free_(void *mem, vl_source_location_t loc) {
    if (!mem) return;
    if (s_enable_logging) {
        if (!s_allocations) {
            s_enable_logging = false;
            s_allocations = VL_DA_INIT(vl_memory_allocation_t);
            s_enable_logging = true;
        }
        s_allocations_count--;
        s_enable_logging = false;
        for (int i = 0; i < VL_DA_LENGTH(s_allocations); i++) {
            if (s_allocations[i].ptr == mem) {
                VL_DA_DELETE(s_allocations, i);
                break;
            }
        }
        s_enable_logging = true;
    }
    free(mem);
}

void vl_memory_print_allocations() {
    if (!s_allocations || (s_allocations && VL_DA_LENGTH(s_allocations) == 0)) {
        printf("no memory allocations yet\n");
        return;
    } 
    printf("total of %zu memory allocations:\n", VL_DA_LENGTH(s_allocations));
    for (int i = 0; i < VL_DA_LENGTH(s_allocations); i++) {
        vl_memory_allocation_t *alloc = s_allocations + i;
        printf("%zu bytes in %s at %s:%i\n", alloc->size, alloc->location.function, alloc->location.file, alloc->location.line);
    }
}

void vl_memory_set_enable_logging(bool logging) {
    s_enable_logging = logging;
}

size_t vl_memory_allocations_count() {
    return s_allocations_count;
}