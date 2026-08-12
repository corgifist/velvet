#include "velvet/css/style.h"
#include "support/result.h"

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