#include "velvet/css/layout.h"

static vl_result_t update_children_style(vl_css_layout_node_t *parent, vl_css_layout_node_t *child);

#ifndef VELVET_CSS_BITS_UPDATE_CHILDREN_STYLE_C
#define VELVET_CSS_BITS_UPDATE_CHILDREN_STYLE_C

static vl_result_t update_children_style(vl_css_layout_node_t *parent, vl_css_layout_node_t *child) {
    parent->size = VL_VEC2_SUB(parent->size, 
        VL_VEC2(parent->effective_padding.w + parent->effective_padding.y, parent->effective_padding.x + parent->effective_padding.z)
    );
    vl_css_layout_node_refresh_style(child);
    parent->size = VL_VEC2_ADD(parent->size, 
        VL_VEC2(parent->effective_padding.w + parent->effective_padding.y, parent->effective_padding.x + parent->effective_padding.z)
    );
    if (child->display == VL_CSS_LAYOUT_DISPLAY_NONE || child->position_type == VL_CSS_LAYOUT_POSITION_ABSOLUTE) {
        return VL_ERROR;
    }
    return VL_SUCCESS;
}

#endif // VELVET_CSS_BITS_UPDATE_CHILDREN_STYLE_C