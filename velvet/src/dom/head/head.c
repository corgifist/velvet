#include "dom/head/head.h"
#include "dom/behavior/div.h"
#include "dom/element.h"
#include "support/memory.h"
#include "support/result.h"

vl_dom_element_t *vl_dom_element_head_new(vl_source_location_t loc) {
    vl_dom_element_funcs_t *funcs = vl_malloc(sizeof(vl_dom_element_funcs_t) + sizeof(vl_dom_element_head_t));
    funcs->free = vl_dom_element_head_free;
    vl_dom_element_head_t *head = VL_PTR_FORWARD(funcs, sizeof(vl_dom_element_funcs_t));
    head->base.tag = "head";
    return (vl_dom_element_t*) head;
}

vl_result_t vl_dom_element_head_free(vl_dom_element_t *element) {
    vl_free(VL_DOM_ELEMENT_FUNCS(element));
    return VL_SUCCESS;
}