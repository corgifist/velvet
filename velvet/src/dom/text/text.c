#include "velvet/dom/text/text.h"
#include "dom/element.h"
#include "support/da.h"
#include "support/memory.h"
#include "support/result.h"
#include "support/managed_assert.h"

vl_dom_element_t *vl_dom_element_text_new(vl_source_location_t loc) {
    vl_dom_element_funcs_t *funcs = vl_malloc(sizeof(vl_dom_element_funcs_t) + sizeof(vl_dom_element_text_t));
    if (!funcs) return NULL;
    funcs->render = vl_dom_element_text_render;
    funcs->set_property = vl_dom_element_text_set_property;
    funcs->get_metric2 = NULL;
    funcs->free = vl_dom_element_text_free;
    vl_dom_element_text_t *element = VL_PTR_FORWARD(funcs, sizeof(*funcs));
    element->text = NULL;
    element->base.children = NULL;
    element->base.owner = NULL;
    element->base.tag = "text";
    return (vl_dom_element_t*) element;
}

vl_result_t vl_dom_element_text_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    printf("text: %s\n", text->text);
    return VL_SUCCESS;
}

vl_result_t vl_dom_element_text_set_property(vl_dom_element_t *element, const char *property, vl_dom_element_property_type_t type, const void *value) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    if (strcmp(property, "innerText") == 0) {
        if (type != VL_DOM_ELEMENT_PROPERTY_STRING) VL_ASSERT(0 && "innerText property requires a STRING");
        if (text->text) {
            VL_DA_FREE(text->text);
        }
        size_t len = strlen(value);
        text->text = VL_DA_INIT_WITH_CAPACITY(char, len + 1);
        text->text[len] = '\0';
        memcpy(text->text, value, len);
        return VL_SUCCESS;
    }
    return VL_ERROR;
}

vl_result_t vl_dom_element_text_free(vl_dom_element_t *element) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    VL_DA_FREE(text->text);
    vl_free(element);
    return VL_SUCCESS;
}