#ifndef VELVET_CSS_STYLESHEET_H
#define VELVET_CSS_STYLESHEET_H

#include "velvet/support/memory.h"
#include "velvet/css/style.h"

struct vl_css_stylesheet {
    VL_DA(vl_css_class_t) classes;
};

typedef struct vl_css_stylesheet vl_css_stylesheet_t;

#define vl_css_stylesheet_init_va_expand(stylesheet, loc, ...) \
    vl_css_stylesheet_init_va_expand(stylesheet, loc)

#define vl_css_stylesheet_init(...) \
    vl_css_stylesheet_init_va_expand(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)
VL_API vl_result_t vl_css_stylesheet_init_(vl_css_stylesheet_t *stylesheet, vl_source_location_t loc);
VL_API vl_result_t vl_css_stylesheet_free(vl_css_stylesheet_t *stylesheet);

#endif // VELVET_CSS_STYLESHEET_H