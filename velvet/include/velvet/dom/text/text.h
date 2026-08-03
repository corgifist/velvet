#ifndef VELVET_DOM_ELEMENT_TEXT_H
#define VELVET_DOM_ELEMENT_TEXT_H

#include "velvet/dom/element.h"
#include "velvet/support/memory.h"
#include "velvet/support/da.h"

struct vl_dom_element_text {
    vl_dom_element_t base;
    VL_DA(char) text;
};

typedef struct vl_dom_element_text vl_dom_element_text_t;

vl_dom_element_t *vl_dom_element_text_new(vl_source_location_t loc);
vl_result_t vl_dom_element_text_free(vl_dom_element_t *element);

#endif // VELVET_DOM_ELEMENT_TEXT_H