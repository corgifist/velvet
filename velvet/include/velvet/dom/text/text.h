#ifndef VELVET_DOM_ELEMENT_TEXT_H
#define VELVET_DOM_ELEMENT_TEXT_H

#include "velvet/dom/render.h"
#include "velvet/support/result.h"
#include "velvet/dom/element.h"
#include "velvet/support/memory.h"
#include "velvet/support/da.h"
#include "velvet/dom/behavior/text.h"

struct vl_dom_element_text {
    vl_dom_element_t base;
    VL_DA(char) text;
    vl_dom_behavior_text_layout_t layout;
    bool rebuild_layout;
};

typedef struct vl_dom_element_text vl_dom_element_text_t;

vl_dom_element_t *vl_dom_element_text_new(vl_source_location_t loc);
vl_result_t vl_dom_element_text_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts);
vl_result_t vl_dom_element_text_set_property(vl_dom_element_t *element, const char *property, vl_dom_element_property_type_t type, const void *value);
vl_vec2_t vl_dom_element_text_get_metric2(vl_dom_element_t *element, vl_dom_element_metric_type_t metric);
vl_result_t vl_dom_element_text_free(vl_dom_element_t *element);

#endif // VELVET_DOM_ELEMENT_TEXT_H