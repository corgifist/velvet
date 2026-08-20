#ifndef VELVET_DOM_ELEMENT_P_H
#define VELVET_DOM_ELEMENT_P_H

#include "dom/render.h"
#include "support/result.h"
#include "velvet/dom/element.h"
#include "velvet/support/memory.h"

struct vl_dom_element_p {
    vl_dom_element_t base;
};

typedef struct vl_dom_element_p vl_dom_element_p_t;

vl_dom_element_t *vl_dom_element_p_new(vl_source_location_t loc);
vl_result_t vl_dom_element_p_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts);
vl_result_t vl_dom_element_p_free(vl_dom_element_t *element);

#endif // VELVET_DOM_ELEMENT_BODY_H