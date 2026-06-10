#include "html/document.h"

vl_result_t vl_html_node_init_move_ownership(vl_html_node_t *node, VL_DA(char) tag) {
    if (!node || !tag) return VL_ERROR;
    node->tag = tag;
    node->attributes = VL_DA_INIT(vl_html_attribute_t);
    node->children = VL_DA_INIT(vl_html_node_t);
    return VL_SUCCESS;
}