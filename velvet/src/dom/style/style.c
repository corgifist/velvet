#include "dom/style/style.h"
#include "css/stylesheet.h"
#include "dom/element.h"
#include "support/da.h"
#include "support/memory.h"
#include "support/result.h"
#include "support/managed_assert.h"

vl_dom_element_t *vl_dom_element_style_new(vl_source_location_t loc) {
    vl_dom_element_funcs_t *funcs = vl_malloc(sizeof(vl_dom_element_funcs_t) + sizeof(vl_dom_element_style_t));
    funcs->set_property = vl_dom_element_style_set_property;
    funcs->free = vl_dom_element_style_free;
    vl_dom_element_style_t *body = VL_PTR_FORWARD(funcs, sizeof(vl_dom_element_funcs_t));
    body->base.tag = "style";
    return (vl_dom_element_t*) body;
}

vl_result_t vl_dom_element_style_set_property(vl_dom_element_t *element, const char *property, vl_dom_element_property_type_t type, const void *value) {
    vl_dom_element_style_t *style = (vl_dom_element_style_t*) element;
    if (strcmp(property, "innerText") == 0) {
        if (type != VL_DOM_ELEMENT_PROPERTY_STRING) VL_ASSERT(0 && "innerText property requires a STRING");
        if (style->text) VL_DA_FREE(style->text);
        style->text = VL_DA_INIT_FROM_STRING(value);
        vl_css_stylesheet_deinit(&style->sheet);
        vl_css_stylesheet_init(&style->sheet, style->text);
        return VL_SUCCESS;
    }
    return VL_ERROR;
}

vl_result_t vl_dom_element_style_free(vl_dom_element_t *element) {
    vl_dom_element_style_t *style = (vl_dom_element_style_t*) element;
    vl_css_stylesheet_deinit(&style->sheet);
    if (style->text) VL_DA_FREE(style->text);
    vl_free(VL_DOM_ELEMENT_FUNCS(element));
    return VL_SUCCESS;
}