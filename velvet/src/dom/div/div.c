#include "dom/div/div.h"
#include "dom/element.h"
#include "support/memory.h"
#include "support/result.h"

vl_dom_element_t *vl_dom_element_div_new(const char *tag, vl_source_location_t loc) {
    vl_dom_element_funcs_t *funcs = vl_malloc(sizeof(vl_dom_element_funcs_t) + sizeof(vl_dom_element_div_t));
    funcs->render = vl_dom_element_div_render;
    funcs->free = vl_dom_element_div_free;
    vl_dom_element_div_t *div = VL_PTR_FORWARD(funcs, sizeof(vl_dom_element_funcs_t));
    div->base.tag = tag;
    return (vl_dom_element_t*) div;
}

vl_result_t vl_dom_element_div_render(vl_dom_element_t *element) {
    if (element->children) {
        for (int i = 0; i < VL_DA_LENGTH(element->children); i++) {
            vl_dom_element_render(element->children[i]);
        }
    }
    return VL_SUCCESS;
}

vl_result_t vl_dom_element_div_free(vl_dom_element_t *element) {
    vl_free(VL_DOM_ELEMENT_FUNCS(element));
    return VL_SUCCESS;
}