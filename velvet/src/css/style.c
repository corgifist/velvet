#include "velvet/css/style.h"
#include "support/da.h"
#include "support/global_error_pool.h"
#include "support/result.h"

vl_result_t vl_css_rule_copy(vl_css_rule_t *dst, const vl_css_rule_t *rule) {
    if (!dst || !rule) return VL_ERROR;
    dst->property = VL_DA_COPY(rule->property);
    dst->value = rule->value;
    return VL_SUCCESS;
}

vl_result_t vl_css_class_merge(vl_css_class_t *dst, const vl_css_class_t *class) {
    if (!dst || !class) return VL_ERROR;
    if (strcmp(dst->name, class->name) != 0) {
        vl_global_error_pool_append("cannot merge two distinct css classes %s and %s", dst->name, class->name);
        return VL_ERROR;
    }
    for (int i = 0; i < VL_DA_LENGTH(class->rules); i++) {
        vl_css_rule_t *rule = class->rules + i;
        vl_css_rule_t *duplicate_rule = NULL;
        for (int j = 0; j < VL_DA_LENGTH(dst->rules); j++) {
            vl_css_rule_t *dst_rule = dst->rules + j;
            if (strcmp(dst_rule->property, rule->property) == 0) {
                duplicate_rule = dst_rule;
                break;
            }
        }
        if (duplicate_rule) {
            vl_css_rule_deinit(rule);
            vl_css_rule_copy(duplicate_rule, rule);
        } else {
            vl_css_rule_t copy_rule = {0};
            vl_css_rule_copy(&copy_rule, rule);
            VL_DA_APPEND(dst->rules, copy_rule);
        }
    }
    return VL_SUCCESS;
}

vl_result_t vl_css_class_copy(vl_css_class_t *dst, const vl_css_class_t *src) {
    if (!dst || !src) return VL_ERROR;
    dst->name = VL_DA_COPY(src->name);
    dst->rules = VL_DA_INIT_WITH_CAPACITY(vl_css_rule_t, VL_DA_LENGTH(src->rules));
    VL_DA_HEADER(dst->rules)->count = VL_DA_LENGTH(src->rules);
    for (int i = 0; i < VL_DA_LENGTH(src->rules); i++) {
        vl_css_rule_copy(dst->rules + i, src->rules + i);
    }
    return VL_SUCCESS;
}

static void print_metric_unit(vl_css_size_metric_t *metric) {
    switch (metric->type) {
    case VL_CSS_SIZE_METRIC_NONE: {
        printf("none");
        break;
    }
    case VL_CSS_SIZE_METRIC_PIXELS: {
        printf("px");
        break;
    }
    }
}

static void print_size_metric(vl_css_size_metric_t *metric) {
    printf("%0.2f", metric->value);
    print_metric_unit(metric);
}

vl_result_t vl_css_value_print(vl_css_value_t value) {
    if (value.repr) {
        printf("%s", value.repr);
        return VL_SUCCESS;
    }
    switch (value.type) {
    case VL_CSS_VALUE_NONE: {
        printf("none");
        return VL_SUCCESS;
    }
    case VL_CSS_VALUE_SIZE_METRIC1: {
        print_size_metric(&value.as.metric1);
        return VL_SUCCESS;
    }
    case VL_CSS_VALUE_SIZE_METRIC4: {
        for (int i = 0; i < 4; i++) {
            print_size_metric(value.as.metric4 + i);
            if (i != 3) printf(" ");
        }
        return VL_SUCCESS;
    }
    case VL_CSS_VALUE_COLOR_RGBA: {
        printf("rgba(%.00f, %.00f, %.00f, %.02f)", value.as.rgba.r, value.as.rgba.g, value.as.rgba.b, value.as.rgba.a);
        return VL_SUCCESS;
    }
    }
    return VL_ERROR;
}

vl_result_t vl_css_rule_print(vl_css_rule_t *rule) {
    if (!rule) return VL_ERROR;
    printf("%s: ", rule->property);
    vl_css_value_print(rule->value);
    printf(";\n");
    return VL_SUCCESS;
}

vl_result_t vl_css_class_print(vl_css_class_t *class) {
    if (!class) return VL_ERROR;
    printf("%s {\n", class->name);
    if (class->rules) {
        for (int i = 0; i < VL_DA_LENGTH(class->rules); i++) {
            printf("\t");
            vl_css_rule_print(class->rules + i);
        }
    }
    printf("}\n");
    return VL_SUCCESS;
}

vl_result_t vl_css_rule_deinit(vl_css_rule_t *rule) {
    if (!rule) return VL_SUCCESS;
    VL_DA_FREE(rule->property);
    return VL_SUCCESS;
}

vl_result_t vl_css_class_deinit(vl_css_class_t *class) {
    if (!class) return VL_ERROR;
    VL_DA_FREE(class->name);
    if (class->rules) {
        for (int i = 0; i < VL_DA_LENGTH(class->rules); i++) {
            vl_css_rule_deinit(class->rules + i);
        }
        VL_DA_FREE(class->rules);
    }
    return VL_SUCCESS;
}