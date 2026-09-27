#include "velvet/css/layout.h"
#include "velvet/support/str.h"

#include "generic_metric_to_metric4.c"
#include "sides.h"

static vl_vec4_t construct_position_metrics(vl_css_layout_node_t *node);

#ifndef VELVET_CSS_CONSTRUCT_POSITION_METRICS_C
#define VELVET_CSS_CONSTRUCT_POSITION_METRICS_C

static vl_vec4_t construct_position_metrics(vl_css_layout_node_t *node) {
    vl_vec4_t metric = VL_VEC4(VL_FLOAT_MIN);
    for (int i = 0; i < VL_DA_LENGTH(node->style.applied_rules); i++) {
        const vl_css_rule_t *rule = node->style.applied_rules[i];
        if (strcmp(rule->property, "inset") == 0) {
            metric = generic_metric_to_metric4(node, NULL, rule->value);
            continue;
        }
        for (int j = 0; j < VL_ARR_LEN(sides); j++) {
            if (strcmp(rule->property, vl_sprintf_tmp("%s", sides[j])) == 0) {
                if (VL_CSS_VALUE_IS_METRIC(rule->value)) {
                    metric.m[j] = vl_css_layout_node_process_metric(
                        node, NULL, rule->value.as.metric1, 
                        (j % 2 == 0) ? node->parent->size.y : node->parent->size.x
                    ).value;
                    goto next;
                }
            }
        }
        next:
        continue;
    }
    // printf("inset: %f %f %f %f\n", metric.x, metric.y, metric.z, metric.w);
    return metric;
}

#endif // VELVET_CSS_CONSTRUCT_POSITION_METRICS_C