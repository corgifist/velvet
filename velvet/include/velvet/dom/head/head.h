#ifndef VELVET_DOM_ELEMENT_HEAD_H
#define VELVET_DOM_ELEMENT_HEAD_H

#include "dom/render.h"
#include "support/result.h"
#include "velvet/dom/element.h"
#include "velvet/support/memory.h"
#include "velvet/support/da.h"

struct vl_dom_element_head {
    vl_dom_element_t base;
};

typedef struct vl_dom_element_head vl_dom_element_head_t;

vl_dom_element_t *vl_dom_element_head_new(vl_source_location_t loc);
vl_result_t vl_dom_element_head_free(vl_dom_element_t *element);

#endif // VELVET_DOM_ELEMENT_HEAD_H