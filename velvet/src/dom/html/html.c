#include "dom/html/html.h"
#include "dom/element.h"
#include "graphics/render.h"
#include "support/memory.h"
#include "support/result.h"
#include "web/web.h"

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
        vl_vec2_t body_size = vl_dom_element_get_metric2(body_element, VL_DOM_ELEMENT_METRIC_SIZE);
        printf("body size: %f %f\n", body_size.x, body_size.y);
        vl_graphics_render_batch_rect(element->owner->owner->render, VL_RECT(VL_VEC2(0, 0), body_size), NULL);
        vl_dom_element_render(body_element, opts);
    }
    return VL_SUCCESS;
}

vl_result_t vl_dom_element_html_free(vl_dom_element_t *element) {
    vl_free(VL_DOM_ELEMENT_FUNCS(element));
    return VL_SUCCESS;
}