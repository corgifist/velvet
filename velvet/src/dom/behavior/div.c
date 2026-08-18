#include "velvet/dom/behavior/div.h"
#include "dom/element.h"
#include "support/result.h"
#include "support/math.h"

vl_result_t vl_dom_behavior_div_render(vl_dom_element_t *parent) {
    if (!parent) return VL_ERROR;
    if (parent->children) {
        for (int i = 0; i < VL_DA_LENGTH(parent->children); i++) {
            vl_dom_element_render(parent->children[i], NULL);
        }
    }
    return VL_SUCCESS;
}

vl_vec2_t vl_dom_behavior_div_get_size(vl_dom_element_t *parent) {
    if (!parent) return VL_VEC2(0, 0);
    vl_vec2_t size = {0};
    if (parent->children) {
        for (int i = 0; i < VL_DA_LENGTH(parent->children); i++) {
            vl_vec2_t child_size = vl_dom_element_get_metric2(parent->children[i], VL_DOM_ELEMENT_METRIC_SIZE);
            size.x = VL_MAX(child_size.x, size.x);
            size.y += child_size.y;
        }
    }
    return size;
}