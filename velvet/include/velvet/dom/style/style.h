#ifndef VELVET_DOM_ELEMENT_STYLE_H
#define VELVET_DOM_ELEMENT_STYLE_H

#include "css/stylesheet.h"
#include "velvet/support/result.h"
#include "velvet/dom/element.h"
#include "velvet/support/memory.h"

struct vl_dom_element_style {
    vl_dom_element_t base;

    VL_DA(char) text;
    vl_css_stylesheet_t sheet;
};

typedef struct vl_dom_element_style vl_dom_element_style_t;

vl_dom_element_t *vl_dom_element_style_new(vl_source_location_t loc);
vl_result_t vl_dom_element_style_set_property(vl_dom_element_t *element, const char *property, vl_dom_element_property_type_t type, const void *value);
vl_result_t vl_dom_element_style_free(vl_dom_element_t *element);

#endif // VELVET_DOM_ELEMENT_HTML_H