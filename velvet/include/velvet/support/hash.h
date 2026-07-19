/*
    hash.h - collection of different hash functions (heavily used in ht.h)
*/

#ifndef VELVET_SUPPORT_HASH_H
#define VELVET_SUPPORT_HASH_H

#include "velvet/common.h"

typedef size_t vl_hash_t;

vl_hash_t vl_hash_combine(vl_hash_t hash1, vl_hash_t hash2);

vl_hash_t vl_hash_string(void *mem, size_t mem_length);
vl_hash_t vl_hash_bytes(void *mem, size_t mem_length);

#endif // VELVET_SUPPORT_HASH_H