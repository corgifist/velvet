#include "dom/void/void.h"
#include "dom/element.h"
#include "support/memory.h"
#include "support/result.h"

vl_dom_element_t *vl_dom_element_void_new(const char *tag, vl_source_location_t loc) {
    vl_dom_element_funcs_t *funcs = vl_malloc(sizeof(vl_dom_element_funcs_t) + sizeof(vl_dom_element_void_t));
    funcs->free = vl_dom_element_void_free;
    vl_dom_element_void_t *element = VL_PTR_FORWARD(funcs, sizeof(vl_dom_element_funcs_t));
    element->base.tag = tag;
    return (vl_dom_element_t*) element;
}

vl_result_t vl_dom_element_void_free(vl_dom_element_t *element) {
    vl_free(VL_DOM_ELEMENT_FUNCS(element));
    return VL_SUCCESS;
}