#include "velvet/support/error_pool.h"
#include "support/global_error_pool.h"
#include "support/result.h"
#include "velvet/support/memory.h"
#include "velvet/support/math.h"
#include <stdarg.h>
#include <stdio.h>
#include <memory.h>

#define VL_ERROR_POOL_INITIAL_CAPACITY 128

#ifdef _MSC_VER
    #define VL_VA_COPY(D, S) ((D) = (S))
#else
    #define VL_VA_COPY(D, S) va_copy(D, S)
#endif

vl_error_pool_t *vl_error_pool_new_(vl_source_location_t loc) {
    vl_error_pool_t *pool = VL_NEW(vl_error_pool_t);
    if (vl_error_pool_init_(pool, loc)) {
        vl_free(pool);
    }
    return pool;
}

vl_result_t vl_error_pool_init_(vl_error_pool_t *pool, vl_source_location_t loc) {
    if (!pool) return VL_ERROR;
    pool->count = pool->__offset = 0;
    pool->__capacity = VL_ERROR_POOL_INITIAL_CAPACITY;
    pool->buffer = vl_malloc(VL_ERROR_POOL_INITIAL_CAPACITY, loc);
    return VL_SUCCESS;
}

static void vl_error_pool_ensure_capacity(vl_error_pool_t *pool, size_t mem_len, vl_source_location_t loc) {
    if (pool->__offset + mem_len > pool->__capacity) {
        size_t new_capacity = VL_MAX(
            pool->__capacity + VL_ERROR_POOL_INITIAL_CAPACITY / 2,
            pool->__capacity + mem_len + mem_len / 2
        );
        pool->buffer = vl_realloc(pool->buffer, new_capacity, loc);
        pool->__capacity = new_capacity;
    }
}

static void *vl_error_pool_push(vl_error_pool_t *pool, void *mem, size_t mem_len, vl_source_location_t loc) {
    if (!pool->buffer) return NULL;
    vl_error_pool_ensure_capacity(pool, mem_len, loc);
    if (mem) memcpy(pool->buffer + pool->__offset, mem, mem_len);
    pool->__offset += mem_len;
    return pool->buffer + pool->__offset - mem_len;
}

vl_result_t vl_error_pool_append_(vl_source_location_t loc, vl_error_pool_t *pool, size_t line, size_t pos, const char *format, ...) {
    if (!pool) return VL_ERROR;
    if (!pool->buffer) {
        if (vl_error_pool_init_(pool, loc)) return VL_ERROR;
    }
    vl_error_pool_push(pool, &line, sizeof(line), loc);
    vl_error_pool_push(pool, &pos, sizeof(pos), loc);
    va_list va1, va2;
    va_start(va1, format);
    VL_VA_COPY(va2, va1);
    size_t formatted_len = vsnprintf(NULL, 0, format, va1);
    vl_error_pool_ensure_capacity(pool, formatted_len + 1, loc);
    char *error_msg = pool->buffer + pool->__offset;
    pool->__offset += vsnprintf(pool->buffer + pool->__offset, pool->__capacity - pool->__offset, format, va2);
    pool->buffer[pool->__offset++] = '\0';
    if (pool == vl_global_error_pool() && (*vl_global_error_pool_feature()) & VL_GLOBAL_ERROR_POOL_LOG_ERROS) {
        printf("%s:%i %s: %s\n", loc.file, loc.line, loc.function, error_msg);
    }
    while (pool->__offset % sizeof(size_t) != 0) {
        vl_error_pool_push(pool, NULL, 1, loc);
    }
    // printf("offset: %zu; formatted_len: %zu\n", pool->__offset, formatted_len);
    va_end(va1);
    va_end(va2);
    return VL_SUCCESS;
}

vl_error_t *vl_error_pool_iterate(vl_error_pool_t *pool, vl_error_t *error) {
    if (!pool || !error) return NULL;
    if (error->__offset >= pool->__offset) return NULL;
    error->line = VL_PTR_DEREF(pool->buffer + error->__offset, size_t);
    error->pos = VL_PTR_DEREF(pool->buffer + error->__offset + sizeof(size_t), size_t);
    error->__offset += 2 * sizeof(size_t);
    char *begin = pool->buffer + error->__offset;
    char *end = begin;
    while (*end++ != '\0' && end < pool->buffer + pool->__capacity) {
        error->__offset++;
    }
    error->__offset++; // skip '\0'
    while (error->__offset % sizeof(size_t) != 0) {
        error->__offset++;
    }
    // printf("error->__offset: %zu\n", error->__offset);
    error->message = begin;
    return error;
}

vl_result_t vl_error_pool_dump(vl_error_pool_t *pool) {
    if (!pool) return VL_ERROR;
    vl_error_t error = VL_ERROR();
    while (vl_error_pool_iterate(pool, &error)) {
        printf("%zu:%zu: %s\n", error.line, error.pos, error.message);
    }
    return VL_SUCCESS;
}

vl_result_t vl_error_pool_deinit(vl_error_pool_t *pool) {
    if (!pool) return VL_ERROR;
    vl_free(pool->buffer);
    memset(pool, 0, sizeof(*pool));
    return VL_SUCCESS;
}

vl_result_t vl_error_pool_free(vl_error_pool_t *pool) {
    if (!pool) return VL_ERROR;
    if (vl_error_pool_deinit(pool)) return VL_ERROR;
    vl_free(pool);
    return VL_SUCCESS;
}