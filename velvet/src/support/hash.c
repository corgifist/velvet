#include "velvet/support/hash.h"
#include "velvet/support/memory.h"

// Source - https://stackoverflow.com/a/27952689
// Posted by Yakk - Adam Nevraumont, modified by community. See post 'Timeline' for change history
// Retrieved 2026-07-19, License - CC BY-SA 4.0

vl_hash_t vl_hash_combine(vl_hash_t lhs, vl_hash_t rhs) {
    if (sizeof(vl_hash_t) >= 8) {
        lhs ^= rhs + 0x517cc1b727220a95 + (lhs << 6) + (lhs >> 2);
    } else {
        lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    }
    return lhs;
}

// REF: https://github.com/haipome/fnv/blob/master/fnv.c
#define FNV_32_PRIME ((uint32_t)0x01000193)
#define FNV_64_PRIME ((uint64_t)0x100000001b3ULL)

#define FNV1_32_INIT ((uint32_t)0x811c9dc5)
#define FNV1_64_INIT ((uint64_t)0xcbf29ce484222325ULL)

static vl_hash_t vl_hash_string64(void *mem, size_t mem_length) {
    unsigned char *s = (unsigned char*) mem;
    vl_hash_t hash = FNV1_64_INIT;

    while (*s) {
        hash *= FNV_64_PRIME;
        hash ^= (uint64_t)*s++;
    }

    return hash;
}

static vl_hash_t vl_hash_string32(void *mem, size_t mem_length) {
    unsigned char *s = (unsigned char*) mem;
    vl_hash_t hash = FNV1_32_INIT;

    while (*s) {
        hash *= FNV_32_PRIME;
        hash ^= (uint32_t)*s++;
    }

    return hash;
}

vl_hash_t vl_hash_string(void *mem, size_t mem_length) {
    if (sizeof(vl_hash_t) >= 8) {
        return vl_hash_string64(mem, mem_length);
    } else {
        return vl_hash_string32(mem, mem_length);
    }
}

static vl_hash_t vl_hash_bytes64(void *mem, size_t mem_length) {
    vl_byte_t *begin = (vl_byte_t*) mem;
    vl_byte_t *end = VL_PTR_FORWARD(mem, mem_length);
    vl_hash_t hash = FNV1_64_INIT;

    while (begin < end) {
        hash *= FNV_64_PRIME;
        hash ^= (uint64_t) *begin++;
    }

    return hash;
}

static vl_hash_t vl_hash_bytes32(void *mem, size_t mem_length) {
    vl_byte_t *begin = (vl_byte_t*) mem;
    vl_byte_t *end = VL_PTR_FORWARD(mem, mem_length);
    vl_hash_t hash = FNV1_32_INIT;

    while (begin < end) {
        hash *= FNV_32_PRIME;
        hash ^= (uint32_t) *begin++;
    }

    return hash;
}

vl_hash_t vl_hash_bytes(void *mem, size_t mem_length) {
    if (sizeof(vl_hash_t) >= 8) {
        return vl_hash_bytes64(mem, mem_length);
    } else {
        return vl_hash_bytes32(mem, mem_length);
    }
}