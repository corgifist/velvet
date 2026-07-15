#ifndef VELVET_HTML_DOCUMENT_H
#define VELVET_HTML_DOCUMENT_H

#include "velvet/support/memory.h"
#include "velvet/support/da.h"
#include "velvet/support/result.h"
#include "velvet/support/api.h"

struct vl_html_attribute {
    VL_DA(char) name; // null-terminated utf-8 string
    VL_DA(char) value; // null-terminated utf-8 string
};

typedef struct vl_html_attribute vl_html_attribute_t;

struct vl_html_node {
    VL_DA(char) tag; // null-terminated utf-8 string
    VL_DA(vl_html_attribute_t) attributes;
    VL_DA(struct vl_html_node) children; // children nodes

    VL_DA(char) text; // null-terminated utf-8 string
};

typedef struct vl_html_node vl_html_node_t;

struct vl_html_document {
    VL_DA(VL_DA_STRING) doctype;
    vl_html_node_t root;
};

typedef struct vl_html_document vl_html_document_t;

VL_API vl_result_t vl_html_attribute_deinit(vl_html_attribute_t *attribute);

VL_API vl_result_t vl_html_node_init(vl_html_node_t *node);
VL_API vl_result_t vl_html_node_print(vl_html_node_t *node);
VL_API vl_result_t vl_html_node_print_with_indent(vl_html_node_t *node, int indent);
VL_API vl_result_t vl_html_node_deinit(vl_html_node_t *node);

VL_API vl_html_document_t *vl_html_document_new(const char *input);
VL_API vl_result_t vl_html_document_init(vl_html_document_t *document, const char *input);
VL_API vl_result_t vl_html_document_print(vl_html_document_t *document);
VL_API vl_result_t vl_html_document_deinit(vl_html_document_t *document);
VL_API vl_result_t vl_html_document_free(vl_html_document_t *document);

#endif // VELVET_DOCUMENT_H