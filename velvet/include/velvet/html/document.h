#ifndef VELVET_HTML_DOCUMENT_H
#define VELVET_HTML_DOCUMENT_H

#include "velvet/support/da.h"
#include "velvet/support/result.h"

struct vl_html_attribute {
    VL_DA(char) name; // null-terminated utf-8 string
    VL_DA(char) value; // null-terminated utf-8 string
};

typedef struct vl_html_attribute vl_html_attribute_t;

struct vl_html_node {
    VL_DA(char) tag; // null-terminated utf-8 string
    VL_DA(vl_html_attribute_t) attributes;
    VL_DA(struct vl_html_node) children; // children nodes

    VL_DA(char) text;
};

typedef struct vl_html_node vl_html_node_t;

vl_result_t vl_html_node_init_move_ownership(vl_html_node_t *node, VL_DA(char) tag);

#endif // VELVET_DOCUMENT_H