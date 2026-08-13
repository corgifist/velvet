#ifndef VELVET_DOM_ELEMENT_HTML_H
#define VELVET_DOM_ELEMENT_HTML_H

#include "velvet/dom/render.h"
#include "velvet/support/result.h"
#include "velvet/dom/element.h"
#include "velvet/support/memory.h"
#include "velvet/support/da.h"

struct vl_dom_element_html {
    vl_dom_element_t base;
};

typedef struct vl_dom_element_html vl_dom_element_html_t;

vl_dom_element_t *vl_dom_element_html_new(vl_source_location_t loc);
vl_result_t vl_dom_element_html_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts);
vl_result_t vl_dom_element_html_free(vl_dom_element_t *element);

#endif // VELVET_DOM_ELEMENT_HTML_H