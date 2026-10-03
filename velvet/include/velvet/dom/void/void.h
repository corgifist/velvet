#ifndef VELVET_DOM_ELEMENT_VOID_H
#define VELVET_DOM_ELEMENT_VOID_H

#include "support/result.h"
#include "velvet/dom/element.h"
#include "velvet/support/memory.h"
#include "velvet/support/da.h"

struct vl_dom_element_void {
    vl_dom_element_t base;
};

typedef struct vl_dom_element_void vl_dom_element_void_t;

vl_dom_element_t *vl_dom_element_void_new(const char *tag, vl_source_location_t loc);
vl_result_t vl_dom_element_void_free(vl_dom_element_t *element);

#endif // VELVET_DOM_ELEMENT_VOID_H