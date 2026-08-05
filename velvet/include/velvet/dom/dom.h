#ifndef VELVET_DOM_H
#define VELVET_DOM_H

#include "velvet/dom/render.h"
#include "velvet/support/result.h"
#include "velvet/dom/element.h"
#include "velvet/html/document.h"

struct vl_web;
struct vl_dom {
    struct vl_web *owner;
    vl_dom_element_t *root;
};

typedef struct vl_dom vl_dom_t;

VL_API vl_result_t vl_dom_init_with_html_document(vl_dom_t *dom, vl_html_document_t *document);
VL_API vl_result_t vl_dom_init_with_html_node(vl_dom_t *dom, vl_html_node_t *node);
VL_API vl_result_t vl_dom_render(vl_dom_t *dom, vl_dom_render_opts_t *opts);

#endif // VELVET_DOM_H