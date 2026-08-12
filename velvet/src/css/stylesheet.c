#include "velvet/css/stylesheet.h"
#include "css/style.h"
#include "support/result.h"

vl_result_t vl_css_stylesheet_init_(vl_css_stylesheet_t *stylesheet, vl_source_location_t loc) {
    if (!stylesheet) return VL_ERROR;
    stylesheet->classes = VL_DA_INIT(vl_css_class_t);
    return VL_SUCCESS;
}

vl_result_t vl_css_stylesheet_free(vl_css_stylesheet_t *stylesheet) {
    if (!stylesheet) return VL_ERROR;
    VL_DA_FREE(stylesheet->classes);
    return VL_SUCCESS;
}