#include "velvet/html/tidy.h"
#include "html/document.h"
#include "support/da.h"
#include "support/result.h"
#include <string.h>

static vl_html_node_t create_minimal_node(const char *tag) {
    vl_html_node_t html_node = {0};
    html_node.tag = VL_DA_INIT_FROM_STRING(tag);
    html_node.children = VL_DA_INIT(vl_html_node_t);
    return html_node;
}

vl_result_t vl_html_tidy_document(vl_html_document_t *document) {
    if (!document) return VL_ERROR;
    if (vl_html_tidy_node(&document->root)) return VL_ERROR;
    if (!document->doctype || (document->doctype && VL_DA_LENGTH(document->doctype) == 0)) {
        if (!document->doctype) {
            document->doctype = VL_DA_INIT_WITH_CAPACITY(VL_DA_STRING, 1);
        }
        *VL_DA_PUSH(document->doctype, VL_DA_STRING) = VL_DA_INIT_FROM_STRING("html");
    }
    return VL_SUCCESS;
}

vl_result_t vl_html_tidy_node(vl_html_node_t *node) {
    if (!node) return VL_ERROR;
    if (node->tag) {
        if (strcmp(node->tag, "html") == 0) {
            bool head_found = false;
            bool body_found = false;
            for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
                if (node->children[i].tag && strcmp(node->children[i].tag, "head") == 0) {
                    head_found = true;
                }
                if (node->children[i].tag && strcmp(node->children[i].tag, "body") == 0) {
                    body_found = true;
                }
            }
            if (head_found && body_found) return VL_SUCCESS;
        }
    }
    if (!node->tag && !node->text && node->children) {
        vl_html_node_t html_node = create_minimal_node("html");
        vl_html_node_t *head_node = NULL;
        vl_html_node_t *body_node = NULL;
        for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
            if (node->children[i].tag && strcmp(node->children[i].tag, "head") == 0) {
                head_node = VL_DA_APPEND(html_node.children, node->children[i]);
                break;
            }
        }
        for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
            if (node->children[i].tag && strcmp(node->children[i].tag, "body") == 0) {
                body_node = VL_DA_APPEND(html_node.children, node->children[i]);
                break;
            }
        }
        if (!head_node) {
            vl_html_node_t empty_head_node = create_minimal_node("head");
            head_node = VL_DA_APPEND(html_node.children, empty_head_node);
        }
        if (!body_node) {
            vl_html_node_t empty_body_node = create_minimal_node("body");
            body_node = VL_DA_APPEND(html_node.children, empty_body_node);
        }
        for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
            if (node->children[i].tag && body_node
                    && (strcmp(node->children[i].tag, "body") != 0)
                    && (strcmp(node->children[i].tag, "head") != 0)) {
                VL_DA_APPEND(body_node->children, node->children[i]);
                break;
            }
        }
        VL_DA_FREE(node->attributes);
        VL_DA_FREE(node->children);
        *node = html_node;
        return VL_SUCCESS;
    }
    if (node->tag && strcmp(node->tag, "head") == 0) {
        vl_html_node_t html_node = create_minimal_node("html");
        vl_html_node_t body_node = create_minimal_node("body");

        VL_DA_APPEND(html_node.children, *node);
        VL_DA_APPEND(html_node.children, body_node);

        *node = html_node;
        return VL_SUCCESS;
    }

    if (node->tag && strcmp(node->tag, "body") == 0) {
        vl_html_node_t html_node = create_minimal_node("html");
        vl_html_node_t head_node = create_minimal_node("head");

        VL_DA_APPEND(html_node.children, head_node);
        VL_DA_APPEND(html_node.children, *node);

        *node = html_node;
        return VL_SUCCESS;
    }

    vl_html_node_t html_node = create_minimal_node("html");

    vl_html_node_t head_node = create_minimal_node("head");
    vl_html_node_t body_node = create_minimal_node("body");

    VL_DA_APPEND(body_node.children, *node);

    VL_DA_APPEND(html_node.children, head_node);
    VL_DA_APPEND(html_node.children, body_node);

    *node = html_node;
    return VL_SUCCESS;
}