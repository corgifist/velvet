#include "dom/body/body.h"
#include "dom/behavior/div.h"
#include "dom/element.h"
#include "support/memory.h"
#include "support/result.h"

vl_dom_element_t *vl_dom_element_body_new(vl_source_location_t loc) {
    vl_dom_element_funcs_t *funcs = vl_malloc(sizeof(vl_dom_element_funcs_t) + sizeof(vl_dom_element_body_t));
    funcs->render = vl_dom_element_body_render;
    funcs->get_metric2 = vl_dom_element_body_get_metric2;
    funcs->free = vl_dom_element_body_free;
    vl_dom_element_body_t *body = VL_PTR_FORWARD(funcs, sizeof(vl_dom_element_funcs_t));
    body->base.tag = "body";
    return (vl_dom_element_t*) body;
}

vl_result_t vl_dom_element_body_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts) {
    vl_dom_behavior_div_render(element);
    return VL_SUCCESS;
}

vl_vec2_t vl_dom_element_body_get_metric2(vl_dom_element_t *element, vl_dom_element_metric_type_t metric) {
    if (!element) return VL_VEC2(0, 0);
    if (metric == VL_DOM_ELEMENT_METRIC_SIZE) {
        return vl_dom_behavior_div_get_size(element);
    }
    return VL_VEC2(0, 0);
}

vl_result_t vl_dom_element_body_free(vl_dom_element_t *element) {
    vl_free(VL_DOM_ELEMENT_FUNCS(element));
    return VL_SUCCESS;
}