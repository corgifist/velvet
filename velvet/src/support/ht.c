#include "support/ht.h"
#include "support/allocator.h"
#include "support/memory.h"
#include "support/managed_assert.h"
#include "support/hash.h"
#include <memory.h>

 void *vl_ht_new(size_t key_size, size_t value_size, size_t capacity, const char *key_type, 
                    vl_ht_hash_func_t hash_func, vl_allocator_t allocator, vl_source_location_t loc) {
    vl_ht_header_t *header = NULL;
    size_t size = sizeof(vl_ht_header_t) + (sizeof(size_t) + sizeof(vl_hash_t) + key_size + value_size) * capacity;
    header = vl_amalloc(allocator, size, loc);
    VL_ASSERT(header && "out of memory (malloc returned NULL)");
    memset(header, 0, size);
    header->magic = VL_HT_MAGIC;
    header->allocator = allocator;
    header->count = 0;
    header->capacity = capacity;
    header->key_size = key_size;
    header->value_size = value_size;
    if (!hash_func) {
        if (strcmp(key_type, "char*") == 0 || strcmp(key_type, "const char*") == 0) {
            header->hash_func = vl_hash_string;
        } else {
            header->hash_func = vl_hash_bytes;
        }
    } else {
        header->hash_func = hash_func;
    }
    return VL_PTR_FORWARD(header, sizeof(vl_ht_header_t));
}

static vl_ht_header_t *header_realloc(vl_ht_header_t *header, size_t new_capacity, vl_source_location_t loc) {
    header->capacity = new_capacity;
    size_t new_size = sizeof(vl_ht_header_t) + (sizeof(size_t) + sizeof(vl_hash_t) + header->key_size + header->value_size) * new_capacity;
    return vl_arealloc(header->allocator, header, new_size, loc);
}

// returns set up entry for specified key
static bool *ht_get_entry(void *ht, void *key, size_t key_size, bool create_new_entry) {
    vl_ht_header_t *header = VL_HT_HEADER(ht);
    vl_hash_t hash = header->hash_func(key, key_size);
    size_t entry_index = hash % header->capacity;
    bool *occupied = VL_HT_ENTRY_AT_INDEX(ht, entry_index);
    if (!*occupied && create_new_entry) {
        // entry is empty
        // putting the data right away
        *occupied = true;
        *((vl_hash_t*) VL_PTR_FORWARD(occupied, sizeof(size_t))) = hash;
        memcpy(VL_PTR_FORWARD(occupied, sizeof(size_t) + sizeof(vl_hash_t)), key, key_size);
        header->count++;
        return occupied;
    } else {
        vl_hash_t saved_hash = *((vl_hash_t*) VL_PTR_FORWARD(occupied, sizeof(size_t)));
        if (saved_hash == hash) {
            // hashes are equal, success
            return occupied;
        } else {
            // hash collision
            // looping until we find an entry
            bool wraparound = false;
            for (size_t i = entry_index; i < header->capacity; i++) {
                bool *entry_occupied = VL_HT_ENTRY_AT_INDEX(ht, i);
                vl_hash_t entry_hash = *((vl_hash_t*) VL_PTR_FORWARD(entry_occupied, sizeof(size_t)));
                if (*entry_occupied) {
                    if (entry_hash == hash) {
                        // found the right entry
                        // only updating the value because key is the same
                        return entry_occupied;
                    } else {
                        // not the entry we're hoping to find
                        // printf("wraparound hash mistmatch\n");
                        goto wraparound;
                    }
                } else if (create_new_entry) {
                    // found an empty entry, success!
                    *entry_occupied = true;
                    *((vl_hash_t*) VL_PTR_FORWARD(entry_occupied, sizeof(size_t))) = hash;
                    memcpy(VL_PTR_FORWARD(entry_occupied, sizeof(size_t) + sizeof(vl_hash_t)), key, key_size);
                    header->count++;
                    return entry_occupied;
                }
                wraparound:
                if (i == header->capacity - 1) {
                    if (wraparound) break;
                    wraparound = true;
                    i = 0;
                }
            }
        }
    }
    return NULL;
}

void *vl_ht_put(void **ht, void *key, void *value, size_t key_size, size_t value_size, vl_source_location_t loc) {
    if (!ht || !*ht || !key) return NULL;
    vl_ht_header_t *header = VL_HT_HEADER(*ht);
    VL_ASSERT(header->magic == VL_HT_MAGIC && "not a hash table");
    VL_ASSERT(header->key_size == key_size && "ht key type mismatch");
    VL_ASSERT(header->value_size && "ht value type mismatch");
    VL_ASSERT(header->hash_func && "ht hash_func not set");
    if (header->count >= header->capacity) {
        header = header_realloc(header, header->capacity << 2, loc);
        *ht = VL_PTR_FORWARD(header, sizeof(vl_ht_header_t));
    }

    bool *entry_ptr = ht_get_entry(*ht, key, key_size, true);
    // set / update entry's value
    memcpy(VL_PTR_FORWARD(entry_ptr, sizeof(size_t) + sizeof(vl_hash_t) + header->key_size), value, value_size);
    
    // unreachable
    return NULL;
}

void *vl_ht_get(void *ht, void *key, size_t key_size) {
    if (!ht || !key) return NULL;
    vl_ht_header_t *header = VL_HT_HEADER(ht);
    VL_ASSERT(header->magic == VL_HT_MAGIC && "not a hash table");
    VL_ASSERT(header->key_size == key_size && "ht key type mismatch");
    VL_ASSERT(header->hash_func && "ht hash_func not set");
    bool *entry_ptr = ht_get_entry(ht, key, key_size, false);
    if (!entry_ptr) return NULL;
    return VL_PTR_FORWARD(entry_ptr, sizeof(size_t) + sizeof(vl_hash_t) + header->key_size);
}

vl_result_t vl_ht_free(void *ht, vl_source_location_t loc) {
    if (!ht) return VL_SUCCESS;
    vl_ht_header_t *header = VL_HT_HEADER(ht);
    VL_ASSERT(header->magic == VL_HT_MAGIC && "not a hash table");
    header->magic = 0;
    vl_afree(header->allocator, header, loc);
    return VL_SUCCESS;
}

vl_ht_entry_t *vl_ht_iterate(void *ht, vl_ht_entry_t *entry) {
    if (!ht || !entry) return NULL;
    vl_ht_header_t *header = VL_HT_HEADER(ht);
    VL_ASSERT(header->magic == VL_HT_MAGIC && "not a hash table");
    if (entry->__index >= header->capacity) return NULL;
    size_t begin_index = entry->__index;
    bool *occupied = VL_HT_ENTRY_AT_INDEX(ht, begin_index);
    if (*occupied) {
        entry->hash = *((vl_hash_t*) VL_PTR_FORWARD(occupied, sizeof(size_t)));
        entry->key = VL_PTR_FORWARD(occupied, sizeof(size_t) + sizeof(vl_hash_t));
        entry->value = VL_PTR_FORWARD(occupied, sizeof(size_t) + sizeof(vl_hash_t) + header->key_size);
        entry->__index++;
        return entry;
    } else {
        while (++begin_index < header->capacity) {
            occupied = VL_HT_ENTRY_AT_INDEX(ht, begin_index);
            if (*occupied) {
                entry->hash = *((vl_hash_t*) VL_PTR_FORWARD(occupied, sizeof(size_t)));
                entry->key = VL_PTR_FORWARD(occupied, sizeof(size_t) + sizeof(vl_hash_t));
                entry->value = VL_PTR_FORWARD(occupied, sizeof(size_t) + sizeof(vl_hash_t) + header->key_size);
                entry->__index = begin_index + 1;
                return entry;
            }
        }
    }
    return NULL;
}