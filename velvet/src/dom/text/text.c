#include "velvet/dom/text/text.h"
#include "dom/element.h"
#include "support/memory.h"
#include "support/result.h"

vl_dom_element_t *vl_dom_element_text_new(vl_source_location_t loc) {
    vl_dom_element_funcs_t *funcs = vl_malloc(sizeof(vl_dom_element_funcs_t) + sizeof(vl_dom_element_text_t));
    if (!funcs) return NULL;
    funcs->get_metric2 = NULL;
    funcs->free = vl_dom_element_text_free;
    vl_dom_element_text_t *element = VL_PTR_FORWARD(funcs, sizeof(*funcs));
    element->text = VL_DA_INIT(char);
    element->base.children = NULL;
    element->base.owner = NULL;
    element->base.tag = "text";
    return (vl_dom_element_t*) element;
}

vl_result_t vl_dom_element_text_free(vl_dom_element_t *element) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    VL_DA_FREE(text->text);
    vl_free(element);
    return VL_SUCCESS;
}