#include "velvet/dom/dom.h"
#include "css/layout.h"
#include "dom/element.h"
#include "dom/render.h"
#include "html/document.h"
#include "support/da.h"
#include "support/global_error_pool.h"
#include "support/result.h"

vl_result_t vl_dom_init_with_html_document(vl_dom_t *dom, vl_html_document_t *document) {
    if (!dom || !document) return VL_ERROR;
    return vl_dom_init_with_html_node(dom, &document->root);
}

vl_dom_element_t *spawn_element(vl_html_node_t *node) {
    if (!node->tag && !node->text) return NULL;
    return vl_dom_element_new(node->text ? "text" : node->tag);
}

vl_dom_element_t *collect_elements(vl_dom_t *owner, vl_dom_element_t *parent, vl_html_node_t *node) {
    if (!node || (!node->tag && !node->text)) return NULL;
    vl_dom_element_t *element = spawn_element(node);
    if (!element) {
        vl_global_error_pool_append("unknown tag '%s' (vl_html_node_t %p)", node->tag, node);
        return NULL;
    }
    element->owner = owner;
    element->parent = parent;
    if (parent) {
        element->layout.parent = &parent->layout;
    }
    if (node->text) {
        vl_dom_element_set_string(element, "innerText", node->text);
    }
    if (node->tag && strcmp(node->tag, "style") == 0) {
        VL_DA(char) css_code = vl_html_node_collect_text(node);
        vl_dom_element_set_string(element, "innerText", css_code);
        VL_DA_FREE(css_code);
    }
    vl_html_attribute_t *class_attribute = vl_html_node_find_attribute(node, "class");
    if (class_attribute && class_attribute->value) {
        vl_dom_element_set_string(element, "className", class_attribute->value);
    }
    if (!node->text && node->children && VL_DA_LENGTH(node->children) != 0) {
        if (!element->children)
            element->children = VL_DA_INIT(vl_dom_element_t*);
        for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
            vl_dom_element_t *child = collect_elements(owner, element, node->children + i);
            if (!child) {
                continue;
            }
            VL_DA_APPEND(element->children, child);
            *VL_DA_PUSH(element->layout.children, vl_css_layout_node_t*) = &child->layout;
        }
    }
    return element;
}

vl_result_t vl_dom_render(vl_dom_t *dom, vl_dom_render_opts_t *opts) {
    if (!dom || !dom->root) return VL_ERROR;
    return vl_dom_element_render(dom->root, opts);
}

vl_result_t vl_dom_init_with_html_node(vl_dom_t *dom, vl_html_node_t *node) {
    if (!dom || !node) return VL_ERROR;
    dom->owner = NULL;
    dom->root = collect_elements(dom, NULL, node);
    return VL_SUCCESS;
}

vl_result_t vl_dom_deinit(vl_dom_t *dom) {
    if (!dom) return VL_ERROR;
    vl_dom_element_free(dom->root);
    dom->root = NULL;
    return VL_SUCCESS;
}