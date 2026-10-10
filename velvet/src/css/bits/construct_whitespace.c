#include "css/style.h"
#include "velvet/css/layout.h"

static vl_css_layout_whitespace_t construct_whitespace(vl_css_layout_node_t *node);

#ifndef VELVET_CSS_BITS_CONSTRUCT_WHITESPACE_C
#define VELVET_CSS_BITS_CONSTRUCT_WHITESPACE_C

static vl_css_layout_whitespace_t construct_whitespace(vl_css_layout_node_t *node) {
    vl_css_layout_whitespace_t result = node->parent->whitespace;
    for (int i = 0; i < VL_DA_LENGTH(node->style.applied_rules); i++) {
        const vl_css_rule_t *rule = node->style.applied_rules[i];
        if (strcmp(rule->property, "text-wrap-mode") == 0) {
            if (VL_CSS_VALUE_IS_LITERAL(rule->value)) {
                if (VL_CSS_VALUE_COMPARE_LITERALS(rule->value, "wrap"))
                    result.text_wrap_mode = VL_CSS_LAYOUT_TEXT_WRAP;
                else if (VL_CSS_VALUE_COMPARE_LITERALS(rule->value, "nowrap"))
                    result.text_wrap_mode = VL_CSS_LAYOUT_TEXT_NOWRAP;
            }
        }
    }
    return result;
}

#endif // VELVET_CSS_BITS_CONSTRUCT_WHITESPACE_C