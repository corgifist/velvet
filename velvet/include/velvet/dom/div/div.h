#ifndef VELVET_DOM_ELEMENT_DIV_H
#define VELVET_DOM_ELEMENT_DIV_H

#include "dom/render.h"
#include "support/result.h"
#include "velvet/dom/element.h"
#include "velvet/support/memory.h"
#include "velvet/support/da.h"

struct vl_dom_element_div {
    vl_dom_element_t base;
};

typedef struct vl_dom_element_div vl_dom_element_div_t;

vl_dom_element_t *vl_dom_element_div_new(vl_source_location_t loc);
vl_result_t vl_dom_element_div_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts);
vl_result_t vl_dom_element_div_free(vl_dom_element_t *element);

#endif // VELVET_DOM_ELEMENT_DIV_H