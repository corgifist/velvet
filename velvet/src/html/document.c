#include "html/document.h"

vl_result_t vl_html_node_init(vl_html_node_t *node) {
    if (!node) return VL_ERROR;
    node->tag = NULL;
    node->attributes = VL_DA_INIT(vl_html_attribute_t);
    node->children = VL_DA_INIT(vl_html_node_t);
    return VL_SUCCESS;
}