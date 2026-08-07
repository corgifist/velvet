/*
    error_pool.h - simple growing array for collecting error
    pass vl_error_pool_t* to _init functions and then collect errors if any
    somewhat similar to StringBuilder in java
*/

#ifndef VELVET_SUPPORT_ERROR_POOL_H
#define VELVET_SUPPORT_ERROR_POOL_H

#include "velvet/common.h"
#include "velvet/support/memory.h"
#include "velvet/support/result.h"
#include "velvet/support/api.h"

struct vl_error_pool;

struct vl_error {
    const char *message;
    size_t line, pos; // error location in code

    size_t __offset; // used internally in vl_error_pool_iterate
};

typedef struct vl_error vl_error_t;

#define VL_ERROR() \
    ((vl_error_t) {0})

struct vl_error_pool {
    char *buffer;
    size_t count;

    size_t __offset, __capacity; // used internally in vl_error_pool_append
};

typedef struct vl_error_pool vl_error_pool_t;

#define VL_ERROR_POOL() \
    ((vl_error_pool_t) {0})


VL_API vl_error_pool_t *vl_error_pool_new_(vl_source_location_t loc);
#define vl_error_pool_new() vl_error_pool_new_(VL_SOURCE_LOCATION_HERE)

/**
 * Initialize the error pool
 * 
 * @remark You don't need to explicitly initialize the error pool,
 . @ref vl_error_pool_append automatically does it when needed for you
 *
 * @return @ref vl_result_t code
 */
VL_API vl_result_t vl_error_pool_init_(vl_error_pool_t *pool, vl_source_location_t loc);
#define vl_error_pool_init(...) vl_error_pool_init_(__VA_ARGS__, VL_SOURCE_LOCATION)


/**
 * Append a formatted error message to the given error pool
 *
 * @remark This function automatically initializes the error pool 
 * if it isn't initialized already
 * @return @ref vl_result_t code
 */
VL_API vl_result_t vl_error_pool_append_(vl_source_location_t loc, vl_error_pool_t *pool, size_t line, size_t pos, const char *format, ...);
#define vl_error_pool_append(...) vl_error_pool_append_(VL_SOURCE_LOCATION_HERE, __VA_ARGS__)

VL_API vl_error_t *vl_error_pool_iterate(vl_error_pool_t *pool, vl_error_t *error);

/**
 * Print all accumulated errors
 * 
 * @return @ref vl_result_t code
 */
VL_API vl_result_t vl_error_pool_dump(vl_error_pool_t *pool);

/**
 * Deinitialize the error pool
 *
 * @remark This invalidates all errors got from this error pool
 * @return @ref vl_result_t code
 */
VL_API vl_result_t vl_error_pool_deinit(vl_error_pool_t *pool);

/**
 * Deinitialize the error pool created by @ref vl_error_pool_new
 *
 * @return @ref vl_result_t code
 */
VL_API vl_result_t vl_error_pool_free(vl_error_pool_t *pool);

#endif // VELVET_SUPPORT_ERROR_POOL_H