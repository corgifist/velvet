#include "support/math.h"
#include "velvet/css/layout.h"
#include "velvet/support/string.h"

#include "generic_metric_to_metric4.c"

static vl_css_size_metric_t select_metric(const vl_css_rule_t *rule, int i1, int i2, int i3);
static void construct_complex_metric(const char *root, vl_vec4_t *result, bool *auto_metric, vl_css_layout_node_t *node);

#ifndef VELVET_CSS_BITS_CONSTRUCT_COMPLEX_METRIC_C
#define VELVET_CSS_BITS_CONSTRUCT_COMPLEX_METRIC_C

static vl_css_size_metric_t select_metric(const vl_css_rule_t *rule, int i1, int i2, int i3) {
    switch (rule->value.type) {
    case VL_CSS_VALUE_SIZE_METRIC1: return rule->value.as.metric1;
    case VL_CSS_VALUE_SIZE_METRIC2: return rule->value.as.metric2[i1];
    case VL_CSS_VALUE_SIZE_METRIC3: return rule->value.as.metric3[i2];
    case VL_CSS_VALUE_SIZE_METRIC4: return rule->value.as.metric4[i3];
    default: return VL_CSS_SIZE_METRIC(0, 0);
    }
}

static void construct_complex_metric(const char *root, vl_vec4_t *result, bool *auto_metric, vl_css_layout_node_t *node) {
    *result = VL_VEC4(0);
    bool important[4] = {0};
    for (int i = 0; i < VL_DA_LENGTH(node->style.applied_rules); i++) {
        const vl_css_rule_t *rule = node->style.applied_rules[i];
        if (!rule->property) continue;
        if (strcmp(rule->property, root) == 0) {
            vl_vec4_t tmp = generic_metric_to_metric4(node, auto_metric, rule->value);
            if (important[0] == rule->important) result->x = tmp.x;
            if (important[1] == rule->important) result->y = tmp.y;
            if (important[2] == rule->important) result->z = tmp.z;
            if (important[3] == rule->important) result->w = tmp.w;
            continue;
        }
        if (strcmp(rule->property, vl_sprintf_tmp("%s-top", root)) == 0) {
            vl_css_size_metric_t metric = select_metric(rule, 0, 0, 0);
            if (important[0] && !rule->important) continue;
            if (rule->important) important[0] = true;
            if (auto_metric) auto_metric[0] = false;
            if (VL_CSS_VALUE_COMPARE_LITERALS(rule->value, "auto") && auto_metric) auto_metric[0] = true;
            else result->x = vl_css_layout_node_process_metric(node, NULL, metric, node->parent->size.y).value;
            continue;
        }
        if (strcmp(rule->property, vl_sprintf_tmp("%s-right", root)) == 0) {
            vl_css_size_metric_t metric = select_metric(rule, 1, 1, 1);
            if (important[1] && !rule->important) continue;
            if (rule->important) important[1] = true;
            if (auto_metric) auto_metric[1] = false;
            if (VL_CSS_VALUE_COMPARE_LITERALS(rule->value, "auto") && auto_metric) auto_metric[1] = true;
            else result->y = vl_css_layout_node_process_metric(node, NULL, metric, node->parent->size.x).value;
            continue;
        }
        if (strcmp(rule->property, vl_sprintf_tmp("%s-bottom", root)) == 0) {
            vl_css_size_metric_t metric = select_metric(rule, 0, 2, 2);
            if (important[2] && !rule->important) continue;
            if (rule->important) important[2] = true;
            if (auto_metric) node->auto_margin[2] = false;
            if (VL_CSS_VALUE_COMPARE_LITERALS(rule->value, "auto") && auto_metric) auto_metric[2] = true;
            else result->z = vl_css_layout_node_process_metric(node, NULL, metric, node->parent->size.y).value;
            continue;
        }
        if (strcmp(rule->property, vl_sprintf_tmp("%s-left", root)) == 0) {
            vl_css_size_metric_t metric = select_metric(rule, 1, 1, 3);
            if (important[3] && !rule->important) continue;
            if (rule->important) important[3] = true;
            if (auto_metric) node->auto_margin[3] = false;
            if (VL_CSS_VALUE_COMPARE_LITERALS(rule->value, "auto") && auto_metric) auto_metric[3] = true;
            else result->w = vl_css_layout_node_process_metric(node, NULL, metric, node->parent->size.x).value;
            continue;
        }
        if (strcmp(rule->property, vl_sprintf_tmp("%s-block-start", root)) == 0) {
            if (auto_metric)  node->auto_margin[0] = false;
            if (important[0] && !rule->important) continue;
            if (rule->important) important[0] = true;
            if (VL_CSS_VALUE_COMPARE_LITERALS(rule->value, "auto") && auto_metric) auto_metric[0] = true;
            else result->x = vl_css_layout_node_process_metric(node, NULL, rule->value.as.metric1, node->parent->size.y).value;
            continue;
        }
        if (strcmp(rule->property, vl_sprintf_tmp("%s-block-end", root)) == 0) {
            if (auto_metric)  node->auto_margin[2] = false;
            if (important[2] && !rule->important) continue;
            if (rule->important) important[2] = true;
            if (VL_CSS_VALUE_COMPARE_LITERALS(rule->value, "auto") && auto_metric) auto_metric[2] = true;
            else result->z = vl_css_layout_node_process_metric(node, NULL, rule->value.as.metric1, node->parent->size.y).value;
            continue;
        }
        if (strcmp(rule->property, vl_sprintf_tmp("%s-inline-start", root)) == 0) {
            if (auto_metric)  node->auto_margin[3] = false;
            if (important[3] && !rule->important) continue;
            if (rule->important) important[3] = true;
            if (VL_CSS_VALUE_COMPARE_LITERALS(rule->value, "auto") && auto_metric) auto_metric[3] = true;
            else result->w = vl_css_layout_node_process_metric(node, NULL, rule->value.as.metric1, node->parent->size.x).value;
            continue;
        }
        if (strcmp(rule->property, vl_sprintf_tmp("%s-inline-end", root)) == 0) {
            if (auto_metric)  node->auto_margin[1] = false;
            if (important[1] && !rule->important) continue;
            if (rule->important) important[1] = true;
            if (VL_CSS_VALUE_COMPARE_LITERALS(rule->value, "auto") && auto_metric) auto_metric[1] = true;
            else result->y = vl_css_layout_node_process_metric(node, NULL, rule->value.as.metric1, node->parent->size.x).value;
            continue;
        }
    }
    if (auto_metric) {
        for (int i = 0; i < 4; i++) {
            if (auto_metric[i]) result->m[i] = 0;
        }
    }
}

#endif // VELVET_CSS_BITS_CONSTRUCT_COMPLEX_METRIC_C