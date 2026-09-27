#include "velvet/css/layout.h"

static void construct_background(vl_css_layout_node_t *node);

#ifndef VELVET_CSS_BITS_CONSTRUCT_BACKGROUND_C
#define VELVET_CSS_BITS_CONSTRUCT_BACKGROUND_C

static void construct_background(vl_css_layout_node_t *node) {
    VL_ZERO_OUT(&node->background);
    for (int i = 0; i < VL_DA_LENGTH(node->style.applied_rules); i++) {
        const vl_css_rule_t *rule = node->style.applied_rules[i];
        if (strcmp(rule->property, "background") == 0) {
            if (rule->value.type == VL_CSS_VALUE_LIST) {
                for (int j = 0; j < VL_DA_LENGTH(rule->value.as.list); j++) {
                    vl_css_value_t *value = rule->value.as.list + j;
                    if (VL_CSS_VALUE_COLOR_COMPATIBLE(*value)) {
                        node->background.color = vl_css_value_to_rgba(*value);
                    }
                }
            }
        }
        if (strcmp(rule->property, "background-color") == 0) {
            if (VL_CSS_VALUE_COLOR_COMPATIBLE(rule->value)) {
                node->background.color = vl_css_value_to_rgba(rule->value);
            }
        }
    }
}

#endif // VELVET_CSS_BITS_CONSTRUCT_BACKGROUND_C