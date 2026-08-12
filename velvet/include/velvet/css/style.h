#ifndef VELVET_CSS_STYLE_H
#define VELVET_CSS_STYLE_H

#include "velvet/common.h"
#include "velvet/support/result.h"
#include "velvet/support/da.h"
#include "velvet/support/ht.h"

enum vl_css_value_type {
    VL_CSS_VALUE_NONE = 0,
    VL_CSS_VALUE_SIZE_METRIC1,
    VL_CSS_VALUE_SIZE_METRIC4
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

struct vl_css_value {
    vl_css_value_type_t type;
    union {
        vl_css_size_metric_t metric1;
        vl_css_size_metric_t metric4[4];
    } as;
};

typedef struct vl_css_value vl_css_value_t;

#define VL_CSS_VALUE_NONE() ((vl_css_value_t) {0})

#define VL_CSS_VALUE_METRIC1(METRIC1) \
    ((vl_css_value_t) {.type = VL_CSS_VALUE_SIZE_METRIC1, .as = {.metric1 = (METRIC1)}})

#define VL_CSS_VALUE_METRIC4(M1, M2, M3, M4) \
    ((vl_css_value_t) {.type = VL_CSS_VALUE_SIZE_METRIC4, .as = {.metric4 = {(M1), (M2), (M3), (M4)}}})

struct vl_css_class;
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

VL_API vl_result_t vl_css_value_print(vl_css_value_t value);
VL_API vl_result_t vl_css_rule_print(vl_css_rule_t *rule);
VL_API vl_result_t vl_css_class_print(vl_css_class_t *class);

VL_API vl_result_t vl_css_rule_deinit(vl_css_rule_t *rule);
VL_API vl_result_t vl_css_class_deinit(vl_css_class_t *class);

#endif // VELVET_CSS_STYLE_H