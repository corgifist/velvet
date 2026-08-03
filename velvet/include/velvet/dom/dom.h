#ifndef VELVET_DOM_H
#define VELVET_DOM_H

#include "velvet/dom/element.h"
#include "velvet/html/document.h"

struct vl_web;
struct vl_dom {
    struct vl_web *owner;
    vl_dom_element_t *root;
};

typedef struct vl_dom vl_dom_t;

VL_API vl_dom_t vl_dom_init_with_html_document(vl_html_document_t document);
VL_API vl_dom_t vl_dom_init_with_html_node(vl_html_node_t node);

#endif // VELVET_DOM_H