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
    if (!node || !node->children) return VL_ERROR;
    for (int i = 0; i < indent; i++) {
        printf("\t");
    }
    if (node->text) {
        printf("%s\n", node->text);
        return VL_SUCCESS;
    }
    printf("<%s", node->tag);
    if (node->attributes) {
        size_t attributes_count = VL_DA_LENGTH(node->attributes);
        if (attributes_count > 0) {
            printf(" ");
        }
        for (int i = 0; i < attributes_count; i++) {
            printf("%s", node->attributes[i].name);
            if (VL_DA_LENGTH(node->attributes[i].value) > 1) {
                printf("=\"%s\"", node->attributes[i].value);
            }
            if (i != attributes_count - 1) {
                printf(" ");
            }
        }
    }
    size_t children_count = VL_DA_LENGTH(node->children);
    if (children_count == 0) {
        printf("/");
    }
    printf(">");
    if (children_count > 0) {
        printf("\n");
    }
    for (int i = 0; i < children_count; i++) {
        if (vl_html_node_print_with_indent(node->children + i, indent + 1)) {
            return VL_ERROR;
        }
    }
    if (children_count > 0) {
        for (int i = 0; i < indent && children_count > 0; i++) {
            printf("\t");
        }
        printf("</%s>", node->tag);
    }
    printf("\n");
    return VL_SUCCESS;
}

vl_result_t vl_html_node_deinit(vl_html_node_t *node) {
    if (!node) return VL_ERROR;
    if (node->tag) VL_DA_FREE(node->tag);
    node->tag = NULL;
    if (node->attributes) {
        for (int i = 0; i < VL_DA_LENGTH(node->attributes); i++) {
            // vl_html_attribute_deinit(node->attributes[i]);
        }
        VL_DA_FREE(node->attributes);
    }
    node->attributes = NULL;
    if (node->children) {
        for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
            vl_html_node_deinit(node->children + i);
        }
        VL_DA_FREE(node->children);
    }
    node->children = NULL;
    if (node->text) VL_DA_FREE(node->text);
    node->text = NULL;
    return VL_SUCCESS;
}