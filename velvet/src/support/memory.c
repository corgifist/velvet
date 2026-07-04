#include "support/memory.h"
#include <stdlib.h>
#include <stdio.h>

static size_t s_allocations_count = 0;

void *vl_malloc_(size_t size, vl_source_location_t loc) {
    printf("allocating %zu bytes in %s at %s:%i\n", size, loc.function, loc.file, loc.line);
    s_allocations_count++;
    return malloc(size);
}

void *vl_realloc_(void *mem, size_t size, vl_source_location_t loc) {
    printf("resizing %p to %zu bytes in %s at %s:%i\n", mem, size, loc.function, loc.file, loc.line);
    return realloc(mem, size);
}

void vl_free_(void *mem, vl_source_location_t loc) {
    if (!mem) return;
    printf("freeing %p in %s at %s:%i\n", mem, loc.function, loc.file, loc.line);
    s_allocations_count--;
    free(mem);
}

size_t vl_memory_allocations_count() {
    return s_allocations_count;
}