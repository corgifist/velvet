#ifndef VELVET_DOM_ELEMENT_SPAN_H
#define VELVET_DOM_ELEMENT_SPAN_H

#include "dom/render.h"
#include "support/result.h"
#include "velvet/dom/element.h"
#include "velvet/support/memory.h"

struct vl_dom_element_span {
    vl_dom_element_t base;
};

typedef struct vl_dom_element_span vl_dom_element_span_t;

vl_dom_element_t *vl_dom_element_span_new(vl_source_location_t loc);
vl_result_t vl_dom_element_span_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts);
vl_result_t vl_dom_element_span_free(vl_dom_element_t *element);

#endif // VELVET_DOM_ELEMENT_BODY_H