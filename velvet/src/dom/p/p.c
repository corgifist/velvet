#include "dom/p/p.h"
#include "dom/behavior/div.h"
#include "dom/element.h"
#include "support/memory.h"
#include "support/result.h"

vl_dom_element_t *vl_dom_element_p_new(vl_source_location_t loc) {
    vl_dom_element_funcs_t *funcs = vl_malloc(sizeof(vl_dom_element_funcs_t) + sizeof(vl_dom_element_p_t));
    funcs->render = vl_dom_element_p_render;;
    funcs->free = vl_dom_element_p_free;
    vl_dom_element_p_t *p = VL_PTR_FORWARD(funcs, sizeof(vl_dom_element_funcs_t));
    p->base.tag = "p";
    return (vl_dom_element_t*) p;
}

vl_result_t vl_dom_element_p_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts) {
    vl_dom_behavior_div_render(element);
    return VL_SUCCESS;
}

vl_result_t vl_dom_element_p_free(vl_dom_element_t *element) {
    vl_free(VL_DOM_ELEMENT_FUNCS(element));
    return VL_SUCCESS;
}