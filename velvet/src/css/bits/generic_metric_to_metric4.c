#include "velvet/css/layout.h"

static vl_vec4_t generic_metric_to_metric4(vl_css_layout_node_t *node, bool *auto_metric, vl_css_value_t value);

#ifndef VELVET_CSS_BITS_GENERIC_METRIC_TO_METRIC4_C
#define VELVET_CSS_BITS_GENERIC_METRIC_TO_METRIC4_C

static vl_vec4_t generic_metric_to_metric4(vl_css_layout_node_t *node, bool *auto_metric, vl_css_value_t value) {
    if (value.type == VL_CSS_VALUE_SIZE_METRIC1) {
        if (value.as.metric1.type == VL_CSS_SIZE_METRIC_AUTO && auto_metric) {
            memset(auto_metric, 1, sizeof(bool) * 4);
            return VL_VEC4(0);
        }
        float mx = vl_css_layout_node_process_metric(node, NULL, value.as.metric1, node->parent->size.x).value;
        float my = vl_css_layout_node_process_metric(node, NULL, value.as.metric1, node->parent->size.y).value;
        return (vl_vec4_t) {my, mx, my, mx};
    }
    if (value.type == VL_CSS_VALUE_SIZE_METRIC2) {
        if (value.as.metric2[0].type == VL_CSS_SIZE_METRIC_AUTO && auto_metric) {
            auto_metric[0] = auto_metric[2] = true;
        }
        if (value.as.metric2[1].type == VL_CSS_SIZE_METRIC_AUTO && auto_metric) {
            auto_metric[1] = auto_metric[3] = true;
        }
        float mx = vl_css_layout_node_process_metric(node, NULL, value.as.metric2[1], node->parent->size.x).value;
        float my = vl_css_layout_node_process_metric(node, NULL, value.as.metric2[0], node->parent->size.y).value;
        return VL_VEC4(my, mx, my, mx);
    }
    if (value.type == VL_CSS_VALUE_SIZE_METRIC3) {
        if (value.as.metric3[0].type == VL_CSS_SIZE_METRIC_AUTO && auto_metric) {
            auto_metric[0] = true;
        }
        if (value.as.metric3[1].type == VL_CSS_SIZE_METRIC_AUTO && auto_metric) {
            auto_metric[1] = auto_metric[3] = true;
        }
        if (value.as.metric3[2].type == VL_CSS_SIZE_METRIC_AUTO && auto_metric) {
            auto_metric[2] = true;
        }
        float mt = vl_css_layout_node_process_metric(node, NULL, value.as.metric3[0], node->parent->size.y).value;
        float mb = vl_css_layout_node_process_metric(node, NULL, value.as.metric3[2], node->parent->size.y).value;
        float mx = vl_css_layout_node_process_metric(node, NULL, value.as.metric3[1], node->parent->size.x).value;
        return VL_VEC4(mt, mx, mb, mx);
    }
    if (value.type == VL_CSS_VALUE_SIZE_METRIC4) {
        if (value.as.metric4[0].type == VL_CSS_SIZE_METRIC_AUTO && auto_metric) {
            auto_metric[0] = true;
        }
        if (value.as.metric4[1].type == VL_CSS_SIZE_METRIC_AUTO && auto_metric) {
            auto_metric[1] = true;
        }
        if (value.as.metric4[2].type == VL_CSS_SIZE_METRIC_AUTO && auto_metric) {
            auto_metric[2] = true;
        }
        if (value.as.metric4[3].type == VL_CSS_SIZE_METRIC_AUTO && auto_metric) {
            auto_metric[3] = true;
        }
        float x = vl_css_layout_node_process_metric(node, NULL, value.as.metric4[0], node->parent->size.y).value;
        float y = vl_css_layout_node_process_metric(node, NULL, value.as.metric4[1], node->parent->size.x).value;
        float z = vl_css_layout_node_process_metric(node, NULL, value.as.metric4[2], node->parent->size.y).value;
        float w = vl_css_layout_node_process_metric(node, NULL, value.as.metric4[3], node->parent->size.x).value;
        return VL_VEC4(x, y, z, w);
    }
    return (vl_vec4_t) {0};
}

#endif // VELVET_CSS_BITS_GENERIC_METRIC_TO_METRIC4_C