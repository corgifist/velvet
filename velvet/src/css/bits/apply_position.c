#include "velvet/css/layout.h"
#include "velvet/web/web.h"

static void apply_position(vl_css_layout_node_t *node);

#ifndef VELVET_CSS_BITS_APPLY_POSITION_C
#define VELVET_CSS_BITS_APPLY_POSITION_C

static void apply_position(vl_css_layout_node_t *node) {
    // printf("%s -> %s (%f %f %f %f)\n", node->tag, parent->tag, node->position.x, node->position.y, parent->position.x, parent->position.y);
    if (node->position_type == VL_CSS_LAYOUT_POSITION_RELATIVE) {
        if (node->position_metrics.x != VL_FLOAT_MIN) {
            node->position.y += node->position_metrics.x;
        } else if (node->position_metrics.z != VL_FLOAT_MIN) {
            node->position.y -= node->position_metrics.z;
        }
        if (node->position_metrics.w != VL_FLOAT_MIN) {
            node->position.x += node->position_metrics.w;
        } else if (node->position_metrics.y != VL_FLOAT_MIN) {
            node->position.x -= node->position_metrics.y;
        }
    }
    if (node->position_type == VL_CSS_LAYOUT_POSITION_ABSOLUTE) {
        vl_css_layout_node_t *parent = node->parent;
        vl_vec2_t parent_size = VL_VEC2();
        vl_vec4_t parent_margin = VL_VEC4();
        if (parent) {
            vl_css_layout_node_t *end = &node->web->root_layout_node;
            while (parent != end) {
                if (!parent->parent) {
                    parent = end;
                    break;
                }
                if (strcmp(parent->tag, "body") == 0) {
                    break;
                }
                if (parent->position_type != VL_CSS_LAYOUT_POSITION_STATIC) {
                    break;
                }
                parent = parent->parent;
            }
            if (strcmp(parent->tag, "body") != 0) {
                parent_size = parent->size;
            } else {
                parent_size = node->web->root_layout_node.size;
            }
            parent_margin = parent->margin;
        } else {
            parent = node;
            parent_size = parent->size;
        }
        if (node->position_metrics.x != VL_FLOAT_MIN && node->position_metrics.z != VL_FLOAT_MIN) {
            node->position.y = node->position_metrics.x + node->margin.x - parent_margin.x - parent_margin.z;
            node->size.y = parent_size.y - (node->position_metrics.x + node->position_metrics.z + node->margin.x + node->margin.z);;
        } else if (node->position_metrics.x != VL_FLOAT_MIN) {
            node->position.y = node->position_metrics.x + node->margin.x - parent_margin.x - parent_margin.z;
        } 
        if (node->position_metrics.y != VL_FLOAT_MIN && node->position_metrics.w != VL_FLOAT_MIN) {
            node->position.x = node->position_metrics.w + node->margin.w - parent_margin.w;
            node->size.x = parent_size.x - (node->position_metrics.y + node->position_metrics.w + node->margin.w + node->margin.y);
        } else if (node->position_metrics.w != VL_FLOAT_MIN) {
            node->position.x = node->position_metrics.w + node->margin.w - parent_margin.w;
        }
    }
}

#endif // VELVET_CSS_BITS_APPLY_POSITION_C