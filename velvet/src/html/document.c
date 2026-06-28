#include "html/document.h"
#include "support/da.h"
#include "support/result.h"

#include <stdio.h>

vl_result_t vl_html_node_init(vl_html_node_t *node) {
    if (!node) return VL_ERROR;
    node->tag = NULL;
    node->attributes = VL_DA_INIT(vl_html_attribute_t);
    node->children = VL_DA_INIT(vl_html_node_t);
    node->text = NULL;
    return VL_SUCCESS;
}

vl_result_t vl_html_node_print(vl_html_node_t *node) {
    return vl_html_node_print_with_indent(node, 0);
}

vl_result_t vl_html_node_print_with_indent(vl_html_node_t *node, int indent) {
    if (!node) return VL_ERROR;
    for (int i = 0; i < indent; i++) {
        printf("\t");
    }
    printf("<%s", node->tag);
    size_t attributes_count = VL_DA_LENGTH(node->attributes);
    if (attributes_count > 0) {
        printf(" ");
    }
    for (int i = 0; i < attributes_count; i++) {
        printf("%s=\"%s\"", node->attributes[i].name, node->attributes[i].value);
        if (i != attributes_count - 1) {
            printf(" ");
        }
    }
    printf(">\n");
    size_t children_count = VL_DA_LENGTH(node->children);
    for (int i = 0; i < children_count; i++) {
        if (vl_html_node_print_with_indent(node->children + i, indent + 1)) {
            return VL_ERROR;
        }
    }
    for (int i = 0; i < indent; i++) {
        printf("\t");
    }
    printf("</%s>\n", node->tag);
    return VL_SUCCESS;
}