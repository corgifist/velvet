#ifndef VELVET_DOM_ELEMENT_BODY_H
#define VELVET_DOM_ELEMENT_BODY_H

#include "dom/render.h"
#include "support/result.h"
#include "velvet/dom/element.h"
#include "velvet/support/memory.h"
#include "velvet/support/da.h"

struct vl_dom_element_body {
    vl_dom_element_t base;
};

typedef struct vl_dom_element_body vl_dom_element_body_t;

vl_dom_element_t *vl_dom_element_body_new(vl_source_location_t loc);
vl_result_t vl_dom_element_body_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts);
vl_vec2_t vl_dom_element_body_get_metric2(vl_dom_element_t *element, vl_dom_element_metric_type_t metric);
vl_result_t vl_dom_element_body_free(vl_dom_element_t *element);

#endif // VELVET_DOM_ELEMENT_BODY_H