#ifndef VELVET_CSS_STYLE_H
#define VELVET_CSS_STYLE_H

#include "velvet/support/memory.h"
#include "velvet/common.h"
#include "velvet/support/result.h"
#include "velvet/support/da.h"
#include "velvet/support/ht.h"

enum vl_css_value_type {
    VL_CSS_VALUE_NONE = 0,
    VL_CSS_VALUE_SIZE_METRIC1,
    VL_CSS_VALUE_SIZE_METRIC4,
    VL_CSS_VALUE_COLOR_RGBA
};

typedef enum vl_css_value_type vl_css_value_type_t;

enum vl_css_size_metric_type {
    VL_CSS_SIZE_METRIC_NONE = 0,
    VL_CSS_SIZE_METRIC_PIXELS
};

typedef enum vl_css_size_metric_type vl_css_size_metric_type_t;

struct vl_css_size_metric {
    vl_css_size_metric_type_t type;
    float value;
};

typedef struct vl_css_size_metric vl_css_size_metric_t;

#define VL_CSS_SIZE_METRIC(TYPE, VALUE) \
    ((vl_css_size_metric_t) {.type = (TYPE), .value = (float) (VALUE)})

struct vl_css_color_rgba {
    union {
        struct {
            float r, g, b, a;
        };
        float m[4];
    };
};

typedef struct vl_css_color_rgba vl_css_color_rgba_t;

#define VL_CSS_COLOR_RGBA(R, G, B, A) \
    ((vl_css_color_rgba_t) {.r = (float) (R), .g = (float) (G), .b = (float) (B), .a = (float) (A)})

struct vl_css_value {
    vl_css_value_type_t type;
    union {
        vl_css_size_metric_t metric1;
        vl_css_size_metric_t metric4[4];
        vl_css_color_rgba_t rgba;
    } as;
};

typedef struct vl_css_value vl_css_value_t;

#define VL_CSS_VALUE(TYPE, ...) \
    ((vl_css_value_t) {.type = (TYPE), .as = __VA_ARGS__})
#define VL_CSS_VALUE_NONE() ((vl_css_value_t) {0})

#define VL_CSS_VALUE_METRIC1(METRIC1) \
    VL_CSS_VALUE(VL_CSS_VALUE_SIZE_METRIC1, {.metric1 = (METRIC1)})

#define VL_CSS_VALUE_METRIC4(M1, M2, M3, M4) \
    VL_CSS_VALUE(VL_CSS_VALUE_SIZE_METRIC4, {.metric4 = {(M1), (M2), (M3), (M4)}})

#define VL_CSS_VALUE_RGBA(R, G, B, A) \
    VL_CSS_VALUE(VL_CSS_VALUE_COLOR_RGBA, {.rgba = VL_CSS_COLOR_RGBA(R, G, B, A)})

struct vl_css_rule {
    VL_DA(char) property;
    vl_css_value_t value;
};

typedef struct vl_css_rule vl_css_rule_t;

struct vl_css_class {
    VL_DA(char) name;
    VL_DA(vl_css_rule_t) rules;
};

typedef struct vl_css_class vl_css_class_t;

struct vl_css_style {
    VL_DA(vl_css_rule_t*) applied_rules;
};

typedef struct vl_css_style vl_css_style_t;

#define vl_css_style_init_va_expand(style, loc, ...) \
    vl_css_style_init_(style, loc)
#define vl_css_style_init(...) \
    vl_css_style_init_va_expand(__VA_ARGS__, VL_HERE)
VL_API vl_result_t vl_css_style_init_(vl_css_style_t *style, vl_source_location_t loc);
VL_API vl_css_value_t vl_css_style_get_property(vl_css_style_t *style, const char *property, vl_css_value_t fallback);

VL_API vl_result_t vl_css_class_merge(vl_css_class_t *dst, const vl_css_class_t *class);
VL_API vl_result_t vl_css_class_copy(vl_css_class_t *dst, const vl_css_class_t *src);

VL_API vl_result_t vl_css_rule_copy(vl_css_rule_t *dst, const vl_css_rule_t *rule);

VL_API vl_result_t vl_css_style_print(vl_css_style_t *style);
VL_API vl_result_t vl_css_class_print(vl_css_class_t *class);
VL_API vl_result_t vl_css_rule_print(vl_css_rule_t *rule);
VL_API vl_result_t vl_css_value_print(vl_css_value_t value);

VL_API vl_result_t vl_css_style_deinit(vl_css_style_t *style);
VL_API vl_result_t vl_css_class_deinit(vl_css_class_t *class);
VL_API vl_result_t vl_css_rule_deinit(vl_css_rule_t *rule);

#endif // VELVET_CSS_STYLE_H