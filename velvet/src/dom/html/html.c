#include "dom/html/html.h"
#include "dom/element.h"
#include "support/memory.h"
#include "support/result.h"

vl_dom_element_t *vl_dom_element_html_new(vl_source_location_t loc) {
    vl_dom_element_funcs_t *funcs = vl_malloc(sizeof(vl_dom_element_funcs_t) + sizeof(vl_dom_element_html_t));
    funcs->render = vl_dom_element_html_render;
    funcs->free = vl_dom_element_html_free;
    vl_dom_element_html_t *body = VL_PTR_FORWARD(funcs, sizeof(vl_dom_element_funcs_t));
    body->base.tag = "html";
    return (vl_dom_element_t*) body;
}

vl_result_t vl_dom_element_html_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts) {
    vl_dom_element_t *body_element = NULL;
    if (element->children) {
        for (int i = 0; i < VL_DA_LENGTH(element->children); i++) {
            if (strcmp(element->children[i]->tag, "body") == 0) {
                body_element = element->children[i];
                break;
            }
        }
    }
    if (body_element) {
        vl_dom_element_render(body_element, opts);
    }
    return VL_SUCCESS;
}

vl_result_t vl_dom_element_html_free(vl_dom_element_t *element) {
    vl_free(VL_PTR_BACKWARD(element, sizeof(vl_dom_element_funcs_t)));
    return VL_SUCCESS;
}