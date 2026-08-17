#include "velvet/css/style.h"
#include "support/da.h"
#include "support/result.h"

vl_result_t vl_css_style_init_(vl_css_style_t *style, vl_source_location_t loc) {
    if (!style) return VL_ERROR;
    style->applied_rules = VL_DA_INIT(vl_css_rule_t*);
    return VL_SUCCESS;
}

vl_css_value_t vl_css_style_get_property(vl_css_style_t *style, const char *property, vl_css_value_t fallback) {
    if (!style || !property) return fallback;
    if (style->applied_rules) {
        for (int i = 0; i < VL_DA_LENGTH(style->applied_rules); i++) {
            vl_css_rule_t *rule = style->applied_rules[i];
            if (rule->property && strcmp(rule->property, property) == 0) {
                return rule->value;
            }
        }
    }
    return fallback;
}

vl_result_t vl_css_style_from_class(vl_css_style_t *style, const vl_css_class_t *class) {
    if (!style || !class) return VL_ERROR;
    if (!style->applied_rules) {
        style->applied_rules = VL_DA_INIT(vl_css_rule_t*);
    }
    if (class->rules) {
        for (int i = 0; i < VL_DA_LENGTH(class->rules); i++) {
            *VL_DA_PUSH(style->applied_rules, vl_css_rule_t*) = class->rules + i;
        }
    }
    return VL_SUCCESS;
}

vl_result_t vl_css_style_merge(vl_css_style_t *dst, const vl_css_style_t *style) {
    if (!dst || !style) return VL_ERROR;
    if (!dst->applied_rules || !style->applied_rules) return VL_ERROR;
    size_t len = VL_DA_LENGTH(style->applied_rules);
    for (int i = 0; i < len; i++) {
        vl_css_rule_t *rule = style->applied_rules[i];
        vl_css_rule_t **duplicate_rule = NULL;
        for (int j = 0; j < VL_DA_LENGTH(dst->applied_rules); j++) {
            vl_css_rule_t *dst_rule = dst->applied_rules[j];
            if (dst_rule->property && strcmp(rule->property, dst_rule->property) == 0) {
                duplicate_rule = dst->applied_rules + j;
                break;
            }
        }
        if (duplicate_rule) {
            vl_css_rule_t *old_rule = *duplicate_rule;
            if ((!old_rule->important && !rule->important) || (old_rule->important && rule->important)) {
                if (rule->priority > old_rule->priority) {
                    *duplicate_rule = rule;
                }
            } else if (!old_rule->important && rule->important) {
                *duplicate_rule = rule;
            } else if (old_rule->important && !rule->important) {
                // we're not merging styles here
            }
        } else {
            VL_DA_APPEND(dst->applied_rules, rule);
        }
    }
    return VL_SUCCESS;
}

vl_result_t vl_css_rule_copy(vl_css_rule_t *dst, const vl_css_rule_t *rule) {
    if (!dst || !rule) return VL_ERROR;
    dst->property = VL_DA_COPY(rule->property);
    dst->value = rule->value;
    dst->priority = rule->priority;
    dst->important = rule->important;
    return VL_SUCCESS;
}

vl_result_t vl_css_class_copy(vl_css_class_t *dst, const vl_css_class_t *src) {
    if (!dst || !src) return VL_ERROR;
    if (src->selectors) {
        dst->selectors = VL_DA_INIT_WITH_CAPACITY(vl_css_class_selector_t, VL_DA_LENGTH(src->selectors));
        for (int i = 0; i < VL_DA_LENGTH(src->selectors); i++) {
            vl_css_class_selector_copy(VL_DA_PUSH(dst->selectors, vl_css_class_selector_t), src->selectors + i);
        }
    }
    if (src->rules) {
        dst->rules = VL_DA_INIT_WITH_CAPACITY(vl_css_rule_t, VL_DA_LENGTH(src->rules));
        for (int i = 0; i < VL_DA_LENGTH(src->rules); i++) {
            vl_css_rule_copy(VL_DA_PUSH(dst->rules, vl_css_rule_t), src->rules + i);
        }
    }
    return VL_SUCCESS;
}

vl_result_t vl_css_class_selector_copy(vl_css_class_selector_t *dst, const vl_css_class_selector_t *selector) {
    if (!dst || !selector) return VL_ERROR;
    if (selector->id_chain) {
        dst->id_chain = VL_DA_INIT_WITH_CAPACITY(vl_css_class_id_t, VL_DA_LENGTH(selector->id_chain));
        for (int i = 0; i < VL_DA_LENGTH(selector->id_chain); i++) {
            vl_css_class_id_copy(VL_DA_PUSH(dst->id_chain, vl_css_class_id_t), selector->id_chain + i);
        }
    }
    return VL_SUCCESS;
}

vl_result_t vl_css_class_id_copy(vl_css_class_id_t *dst, const vl_css_class_id_t *id) {
    if (!dst || !id) return VL_ERROR;
    dst->type = id->type;
    dst->name = VL_DA_COPY(id->name);
    return VL_SUCCESS;
}

static void print_rule(vl_css_rule_t *rule) {
    printf("%i %s: ", rule->priority, rule->property);
    vl_css_value_print(rule->value);
    if (rule->important) {
        printf(" !important");
    }
    printf(";");
}

vl_result_t vl_css_style_print(vl_css_style_t *style) {
    if (!style) return VL_ERROR;
    if (style->applied_rules) {
        size_t len = VL_DA_LENGTH(style->applied_rules);
        for (int i = 0; i < len; i++) {
            vl_css_rule_t *rule = style->applied_rules[i];
            print_rule(rule);
            if (i != len - 1) printf(" ");
        }
        printf("\n");
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
    print_rule(rule);
    printf("\n");
    return VL_SUCCESS;
}


static void print_class_id(vl_css_class_id_t *id) {
    if (!id) return;
    switch (id->type) {
    case VL_CSS_CLASS_ID_ALL: {
        printf("*");
        return;
    }
    case VL_CSS_CLASS_ID_ELEMENT: break;
    case VL_CSS_CLASS_ID_CLASS: {
        printf(".");
        break;
    }
    }
    printf("%s", id->name);
}

static void print_class_selector(vl_css_class_selector_t *selector) {
    if (!selector->id_chain) return;
    size_t len = VL_DA_LENGTH(selector->id_chain);
    for (int i = 0; i < len; i++) {
        print_class_id(selector->id_chain + i);
        if (i != len - 1) printf(" ");
    }
}

vl_result_t vl_css_class_print(vl_css_class_t *class) {
    if (!class) return VL_ERROR;
    if (class->selectors) {
        size_t len = VL_DA_LENGTH(class->selectors);
        for (int i = 0; i < len; i++) {
            print_class_selector(class->selectors + i);
            if (i != len - 1) printf(", ");
        }
    } else {
        printf("unknown selector");
    }
    printf(" {\n");
    if (class->rules) {
        for (int i = 0; i < VL_DA_LENGTH(class->rules); i++) {
            printf("\t");
            vl_css_rule_print(class->rules + i);
        }
    }
    printf("}\n");
    return VL_SUCCESS;
}

vl_result_t vl_css_class_selector_print(vl_css_class_selector_t *selector) {
    if (!selector) return VL_ERROR;
    print_class_selector(selector);
    printf("\n");
    return VL_SUCCESS;
}

vl_result_t vl_css_class_id_print(vl_css_class_id_t *id) {
    if (!id) return VL_ERROR;
    print_class_id(id);
    printf("\n");
    return VL_SUCCESS;
}

vl_result_t vl_css_class_selector_deinit(vl_css_class_selector_t *selector) {
    if (!selector) return VL_ERROR;
    if (selector->id_chain) {
        for (int i = 0; i < VL_DA_LENGTH(selector->id_chain); i++) {
            vl_css_class_id_deinit(selector->id_chain + i);
        }
        VL_DA_FREE(selector->id_chain);
    }
    return VL_SUCCESS;
}

vl_result_t vl_css_class_id_deinit(vl_css_class_id_t *id) {
    if (!id) return VL_ERROR;
    VL_DA_FREE(id->name);
    return VL_SUCCESS;
}

vl_result_t vl_css_style_deinit(vl_css_style_t *style) {
    if (!style) return VL_ERROR;
    VL_DA_FREE(style->applied_rules);
    return VL_SUCCESS;
}

vl_result_t vl_css_rule_deinit(vl_css_rule_t *rule) {
    if (!rule) return VL_SUCCESS;
    VL_DA_FREE(rule->property);
    return VL_SUCCESS;
}

vl_result_t vl_css_class_deinit(vl_css_class_t *class) {
    if (!class) return VL_ERROR;
    if (class->selectors) {
        for (int i = 0; i < VL_DA_LENGTH(class->selectors); i++) {
            vl_css_class_selector_deinit(class->selectors + i);
        }
        VL_DA_FREE(class->selectors);
    }
    if (class->rules) {
        for (int i = 0; i < VL_DA_LENGTH(class->rules); i++) {
            vl_css_rule_deinit(class->rules + i);
        }
        VL_DA_FREE(class->rules);
    }
    return VL_SUCCESS;
}