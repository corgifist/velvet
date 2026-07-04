#include "html/document.h"
#include "html/parser.h"

#include "html/tidy.h"
#include "support/da.h"
#include "support/result.h"

#include <stdio.h>

vl_result_t vl_html_attribute_deinit(vl_html_attribute_t *attribute) {
    if (!attribute) return VL_ERROR;
    VL_DA_FREE(attribute->name);
    VL_DA_FREE(attribute->value);
    return VL_SUCCESS;
}

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
            vl_html_attribute_deinit(node->attributes + i);
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

vl_result_t vl_html_document_init(vl_html_document_t *document, const char *input) {
    if (!document || !input) return VL_ERROR;
    document->doctype = VL_DA_INIT(VL_DA_STRING);
    vl_html_node_t root_node = {0};
    vl_html_node_t tmp_node = {0};
    if (vl_html_node_init(&tmp_node)) return VL_ERROR;
    vl_html_parser_t parser = {0};
    if (vl_html_parser_init(&parser, input)) {
        goto failure;
    }
    vl_result_t parse_result = VL_HTML_PARSER_STOP;
    int collector_state = 0;
    vl_html_node_t prev_node = {0};
    while ((parse_result = vl_html_parser_get_ex(&parser, &tmp_node)) != VL_HTML_PARSER_STOP) {
        if (parse_result == VL_ERROR) {
            goto failure;
        }
        if (parse_result == VL_HTML_PARSER_DOCTYPE_NODE) {
            for (int i = 0; i < VL_DA_LENGTH(tmp_node.attributes); i++) {
                VL_DA_APPEND(document->doctype, tmp_node.attributes[i].value);
            }
            VL_DA_FREE(tmp_node.attributes);
            tmp_node.attributes = NULL;
            if (vl_html_node_deinit(&tmp_node)) {
                goto failure;
            }
            if (vl_html_node_init(&tmp_node)) {
                goto failure;
            }
            continue;
        }
        if (collector_state == 0) {
            prev_node = tmp_node;
            if (vl_html_node_init(&tmp_node)) {
                goto failure;
            }
            collector_state = 1;
            continue;
        }
        if (collector_state == 1 || collector_state == 2) {
            if (collector_state == 1 && vl_html_node_init(&root_node)) {
                goto failure;
            }
            if (collector_state == 1) {
                VL_DA_APPEND(root_node.children, prev_node);
            }
            VL_DA_APPEND(root_node.children, tmp_node);
            if (vl_html_node_init(&tmp_node)) {
                goto failure;
            }
            collector_state = 2;
        }
    }
    if (collector_state == 1) {
        document->root = prev_node;
    } else {
        document->root = root_node;
    }
    if (vl_html_node_deinit(&tmp_node)) goto failure;
    if (vl_html_tidy_node(&document->root)) {
        goto failure;
    }
    vl_html_parser_deinit(&parser);
    return VL_SUCCESS;

    failure:
    VL_DA_FREE(document->doctype);
    vl_html_parser_deinit(&parser);
    vl_html_node_deinit(&tmp_node);
    vl_html_node_deinit(&root_node);
    return VL_ERROR;
}

vl_result_t vl_html_document_print(vl_html_document_t *document) {
    if (!document) return VL_ERROR;
    if (document->doctype) {
        printf("<!doctype ");
        for (int i = 0; i < VL_DA_LENGTH(document->doctype); i++) {
            printf("%s", document->doctype[i]);
            if (i != VL_DA_LENGTH(document->doctype) - 1) {
                printf(" ");
            }
        }
        printf(">\n");
    }
    if (vl_html_node_print(&document->root)) return VL_ERROR;
    return VL_SUCCESS;
}

vl_result_t vl_html_document_deinit(vl_html_document_t *document) {
    if (!document) return VL_ERROR;
    vl_html_node_deinit(&document->root);
    if (document->doctype) {
        for (int i = 0; i < VL_DA_LENGTH(document->doctype); i++) {
            VL_DA_FREE(document->doctype[i]);
        }
        VL_DA_FREE(document->doctype);
    }
    return VL_SUCCESS;
}