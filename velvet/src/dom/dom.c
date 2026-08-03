#include "velvet/dom/dom.h"
#include "dom/element.h"
#include "html/document.h"
#include "support/da.h"

vl_dom_t vl_dom_init_with_html_document(vl_html_document_t document) {
    return vl_dom_init_with_html_node(document.root);
}

vl_dom_element_t *spawn_element(vl_html_node_t *node) {
    if (!node->tag && !node->text) return NULL;
    return vl_dom_element_new(node->text ? "text" : node->tag);
}

vl_dom_element_t *collect_elements(vl_html_node_t *node) {
    if (!node || (!node->tag && !node->text)) return NULL;
    vl_dom_element_t *element = spawn_element(node);
    if (!element) return NULL;
    if (!node->text && node->children && VL_DA_LENGTH(node->children) > 0) {
        if (!element->children)
            element->children = VL_DA_INIT(vl_dom_element_t*);
        for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
            vl_dom_element_t *child = spawn_element(node->children + i);
            if (!child) {
                for (int j = 0; j < VL_DA_LENGTH(element->children); j++) {
                    vl_dom_element_free(element->children[j]);
                }
                vl_dom_element_free(element);
                return NULL;
            }
            VL_DA_APPEND(element->children, child);
        }
    }
    return element;
}

vl_dom_t vl_dom_init_with_html_node(vl_html_node_t node) {
    vl_dom_t dom = {0};
    dom.root = collect_elements(&node);
    return dom;
}