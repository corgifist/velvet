#include "velvet/css/layout.h"
#include "velvet/html/tags.h"

static void construct_dimensions(vl_css_layout_node_t *node);

#ifndef VELVET_CSS_BITS_CONSTRUCT_DIMENSIONS_C
#define VELVET_CSS_BITS_CONSTRUCT_DIMENSIONS_C

static void construct_dimensions(vl_css_layout_node_t *node) {
    vl_vec2_t dimensions = {VL_FLOAT_MIN, VL_FLOAT_MIN};
    bool fit_width = (node->position_type == VL_CSS_LAYOUT_POSITION_ABSOLUTE);
    node->allow_width_growth = vl_html_is_tag_inline(node->tag) && !fit_width;
    VL_ZERO_OUT(node->lock_dimensions, sizeof(node->lock_dimensions));
    if (VL_CSS_VALUE_COMPARE_LITERALS(node->display, "block")) {
        vl_css_value_t width_value = vl_css_layout_node_get_property(node, "width", VL_CSS_VALUE_NONE());
        if (VL_CSS_VALUE_IS_METRIC(width_value)) {
            vl_css_size_metric_t width_metric = vl_css_layout_node_process_metric(node, "width", width_value.as.metric1, node->parent->size.x);
            dimensions.x = width_metric.value;
            node->lock_dimensions[0] = true;
        } else if (VL_CSS_VALUE_COMPARE_LITERALS(width_value, "fit-content") || fit_width) {
            dimensions.x = 0;
            node->allow_width_growth = true;
        }
        vl_css_value_t height_value = vl_css_layout_node_get_property(node, "height", VL_CSS_VALUE_CONST_LITERAL("fit-content"));
        if (VL_CSS_VALUE_IS_METRIC(height_value)) {
            vl_css_size_metric_t height_metric = vl_css_layout_node_process_metric(node, "height", height_value.as.metric1, node->parent->size.y);
            dimensions.y = height_metric.value;
            node->lock_dimensions[1] = true;
        } else if (VL_CSS_VALUE_COMPARE_LITERALS(height_value, "fit-content")) {
            dimensions.y = 0;
        }
    }
    node->raw_dimensions = dimensions;
}

#endif // VELVET_CSS_BITS_CONSTRUCT_DIMENSIONS_C