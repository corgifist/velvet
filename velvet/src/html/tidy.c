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

vl_result_t vl_html_tidy_node(vl_html_node_t *node) {
    if (!node) return VL_ERROR;
    if (!node->tag) {
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
    if (strcmp(node->tag, "head") == 0) {
        vl_html_node_t html_node = create_minimal_node("html");
        vl_html_node_t body_node = create_minimal_node("body");

        VL_DA_APPEND(html_node.children, *node);
        VL_DA_APPEND(html_node.children, body_node);

        *node = html_node;
        return VL_SUCCESS;
    }

    if (strcmp(node->tag, "body") == 0) {
        vl_html_node_t html_node = create_minimal_node("html");
        vl_html_node_t head_node = create_minimal_node("head");

        VL_DA_APPEND(html_node.children, head_node);
        VL_DA_APPEND(html_node.children, *node);

        *node = html_node;
        return VL_SUCCESS;
    }

    if (strcmp(node->tag, "html") != 0) {
        vl_html_node_t html_node = create_minimal_node("html");

        vl_html_node_t head_node = create_minimal_node("head");
        vl_html_node_t body_node = create_minimal_node("body");

        VL_DA_APPEND(body_node.children, *node);

        VL_DA_APPEND(html_node.children, head_node);
        VL_DA_APPEND(html_node.children, body_node);

        *node = html_node;
        return VL_SUCCESS;
    }

    return VL_SUCCESS;
}