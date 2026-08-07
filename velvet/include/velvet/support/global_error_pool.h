#ifndef VELVET_SUPPORT_GLOBAL_ERROR_POOL_H
#define VELVET_SUPPORT_GLOBAL_ERROR_POOL_H

#include "velvet/support/error_pool.h"

enum vl_global_error_pool_feature {
    VL_GLOBAL_ERROR_POOL_LOG_ERROS = 1
};

typedef enum vl_global_error_pool_feature vl_global_error_pool_feature_t;

VL_API vl_error_pool_t *vl_global_error_pool();
VL_API vl_global_error_pool_feature_t *vl_global_error_pool_feature();

#define vl_global_error_pool_append(...) \
    vl_error_pool_append(vl_global_error_pool(), 0, 0, __VA_ARGS__)

#endif // VELVET_SUPPORT_GLOBAL_ERROR_POOL_H