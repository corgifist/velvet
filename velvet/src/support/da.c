#include "support/da.h"
#include "support/allocator.h"
#include "support/managed_assert.h"
#include "support/memory.h"
#include <string.h>

static void vl_da_grow(vl_da_header_t **hp, vl_source_location_t loc, vl_allocator_t *allocator) {
    if (!hp || !*hp) return;
    vl_da_header_t *header = *hp;
    // https://rcoh.me/posts/notes-on-cpython-list-internals/
    header->capacity = (size_t) header->capacity + (header->capacity >> 3) +
        (header->capacity < 9 ? 3 : 6);
    *hp = vl_arealloc(*allocator, header, sizeof(vl_da_header_t) + header->element_size * header->capacity, loc);
}

static void *vl_da_shrink(VL_DA(void) da, vl_source_location_t loc, vl_allocator_t *allocator) {
    if (!da) return NULL;
    vl_da_header_t *header = VL_DA_HEADER(da);
    header->capacity /= 2;
    header = vl_arealloc(*allocator, header, sizeof(vl_da_header_t) + header->element_size * header->capacity, loc);
    return VL_PTR_FORWARD(header, sizeof(vl_da_header_t));
}

void *vl_da_init(size_t element_size, size_t capacity, vl_source_location_t loc, vl_allocator_t allocator) {
    vl_byte_t *mem;
    mem = vl_amalloc(allocator, sizeof(vl_da_header_t) + element_size * capacity, loc);
    if (!mem) return NULL;
    memset(mem, 0, sizeof(vl_da_header_t) + element_size * capacity);
    vl_da_header_t *header = (vl_da_header_t*) mem;
    header->count = 0;
    header->capacity = capacity;
    header->element_size = element_size;
    header->allocator = allocator;
    header->magic = VL_DA_MAGIC;
    return mem + sizeof(vl_da_header_t);
}

void *vl_da_init_from_string(const char *string, vl_source_location_t loc, vl_allocator_t allocator) {
    VL_ASSERT(string && "const char *string is NULL");
    unsigned long length = strlen(string);
    VL_DA(char) da = vl_da_init(1, length + 1, loc, allocator);
    if (!da) return NULL;
    memcpy(da, string, length);
    da[length] = '\0';
    return da;
}

void *vl_da_append(VL_DA(void) *da, void *item, size_t element_size, vl_source_location_t loc) {
    if (!da || !*da) return NULL;
    VL_ASSERT(VL_DA_MAGIC_MATCHES(*da));
    vl_da_header_t *header = VL_DA_HEADER(*da);
    if (item) VL_ASSERT((element_size == header->element_size) && "mismatching DA types");
    while (header->count >= header->capacity) {
        vl_da_grow(&header, loc, &header->allocator);
        *da = VL_PTR_FORWARD(header, sizeof(vl_da_header_t));
        VL_ASSERT(*da && "couldn't grow da in vl_da_append"); 
    }
    void *target_ptr = VL_PTR_FORWARD(*da, header->element_size * header->count++);
    if (item) memcpy(target_ptr, item, element_size);
    return target_ptr;
}

void vl_da_delete(VL_DA(void) *da, size_t index, vl_source_location_t loc) {
    if (!da || !*da) return;
    VL_ASSERT(VL_DA_MAGIC_MATCHES(*da));
    vl_da_header_t *header = VL_DA_HEADER(*da);
    VL_ASSERT((index >= 0 && index < header->count) && "out of bounds call to vl_da_delete");
    if (index == header->count - 1) {
        memset(VL_PTR_FORWARD(*da, header->element_size * (header->count - 1)), 0, header->element_size);
    } else {
        for (int i = index + 1; i < header->count; i++) {
            memmove(VL_PTR_FORWARD(*da, header->element_size * (i - 1)),
                    VL_PTR_FORWARD(*da, header->element_size * i),
                    header->element_size);
        }
    }
    header->count--;
    if (header->count <= header->capacity / 2 && header->capacity / 2 >= VL_DA_DEFAULT_CAPACITY) {
        *da = vl_da_shrink(*da, loc, &header->allocator);
        VL_ASSERT(VL_DA_HEADER(*da) && "couldn't shrink da in vl_da_delete");
    }
}

void vl_da_free(VL_DA(void) *da, vl_source_location_t loc) {
    if (!da || !*da) return;
    vl_da_header_t *header = VL_DA_HEADER(*da);
    if (!header) return;
    VL_ASSERT(VL_DA_MAGIC_MATCHES(*da));
    vl_afree(header->allocator, header);
    *da = NULL;
}

void vl_da_dump_header(vl_da_header_t *header) {
    printf("-- header: %p --\n", header);
    if (!header) {
        printf("header is NULL\n");
        printf("---------------------\n");
        return;
    }
    printf("magic: %llu == %llu\n", header->magic, VL_DA_MAGIC);
    printf("count: %zu\n", header->count);
    printf("capacity: %zu\n", header->capacity);
    printf("element size: %zu\n", header->element_size);
    printf("---------------------\n");
}