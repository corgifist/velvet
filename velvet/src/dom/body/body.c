#include "dom/body/body.h"
#include "dom/element.h"
#include "support/memory.h"
#include "support/result.h"

vl_dom_element_t *vl_dom_element_body_new(vl_source_location_t loc) {
    vl_dom_element_funcs_t *funcs = vl_malloc(sizeof(vl_dom_element_funcs_t) + sizeof(vl_dom_element_body_t));
    funcs->render = vl_dom_element_body_render;
    funcs->free = vl_dom_element_body_free;
    vl_dom_element_body_t *body = VL_PTR_FORWARD(funcs, sizeof(vl_dom_element_funcs_t));
    body->base.tag = "body";
    return (vl_dom_element_t*) body;
}

vl_result_t vl_dom_element_body_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts) {
    for (int i = 0; i < VL_DA_LENGTH(element->children); i++) {
        vl_dom_element_render(element->children[i], opts);
    }
    return VL_SUCCESS;
}

vl_result_t vl_dom_element_body_free(vl_dom_element_t *element) {
    vl_free(VL_PTR_BACKWARD(element, sizeof(vl_dom_element_funcs_t)));
    return VL_SUCCESS;
}