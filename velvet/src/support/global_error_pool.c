#include "support/global_error_pool.h"
#include "support/error_pool.h"

vl_error_pool_t *vl_global_error_pool() {
    static vl_error_pool_t s_ep = {0};
    return &s_ep;
}

vl_global_error_pool_feature_t *vl_global_error_pool_feature() {
    static vl_global_error_pool_feature_t s_feature = VL_GLOBAL_ERROR_POOL_LOG_ERROS;
    return &s_feature;
}