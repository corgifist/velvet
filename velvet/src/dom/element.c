#include "velvet/dom/element.h"
#include "support/memory.h"
#include "velvet/support/feature.h"
#include <string.h>

#if VL_FEATURE(DOM_TEXT_NODE)
    #include "velvet/dom/text/text.h"
#endif // VL_FEATURE(DOM_TEXT_NODE)

typedef struct {
    const char *tag;
    vl_dom_element_new_func new_; // just to make sure we don't collide with the C++ new
} vl_dom_element_pair_t;

static const vl_dom_element_pair_t s_elements[] = {
#if VL_FEATURE(DOM_TEXT_NODE)
    {"text", vl_dom_element_text_new},
#endif // VL_FEATURE(DOM_TEXT_NODE)
};

vl_dom_element_t *vl_dom_element_new_(const char *tag, vl_source_location_t loc) {
    for (int i = 0; i < VL_ARR_SIZE(s_elements); i++) {
        if (strcmp(s_elements[i].tag, tag) == 0) {
            return s_elements[i].new_(loc);
        }
    }
    return NULL;
}

vl_result_t vl_dom_element_free(vl_dom_element_t *element) {
    if (!element) return VL_ERROR;
    if (element->children) {
        for (int i = 0; i < VL_DA_LENGTH(element->children); i++) {
            vl_dom_element_free(element->children[i]);
        }
        VL_DA_FREE(element->children);
    }
    return VL_DOM_ELEMENT_FUNCS(element)->free(element);
}