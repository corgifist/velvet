#ifndef VELVET_CSS_LAYOUT_H
#define VELVET_CSS_LAYOUT_H

#include "velvet/common.h"
#include "velvet/css/style.h"
#include "velvet/css/stylesheet.h"
#include "velvet/graphics/geometry.h"

typedef void* vl_css_layout_node_owner_t;

struct vl_css_layout_node;
struct vl_web;
typedef vl_vec2_t (*vl_css_layout_node_get_content_size)(struct vl_css_layout_node *node);
struct vl_css_layout_node {
    struct vl_web *web;
    vl_css_layout_node_owner_t owner;
    struct vl_css_layout_node *parent;
    VL_DA(struct vl_css_layout_node*) children;
    vl_css_stylesheet_t *stylesheet;

    vl_css_layout_node_get_content_size get_content_size;
    const char *tag;

    vl_css_class_selector_t tag_selector;
    VL_DA(vl_css_class_selector_t) affecting_selectors;
    vl_css_style_t style;

    vl_css_value_t display;
    vl_vec2_t size;
    vl_vec2_t position;
    vl_vec4_t margin;
    vl_vec4_t padding;
    float block_last_margin;
    float span_y_offset;
    bool calculating_layout;
};

typedef struct vl_css_layout_node vl_css_layout_node_t;

VL_API vl_result_t vl_css_layout_node_init(vl_css_layout_node_t *node, const char *tag);
VL_API vl_result_t vl_css_layout_node_refresh_style(vl_css_layout_node_t *node);
VL_API vl_result_t vl_css_layout_node_process(vl_css_layout_node_t *node);
VL_API vl_vec2_t vl_css_layout_node_get_raw_content_size(vl_css_layout_node_t *node);
VL_API vl_css_value_t vl_css_layout_node_get_property(vl_css_layout_node_t *node, const char *property, vl_css_value_t fallback);
VL_API vl_css_size_metric_t vl_css_layout_node_process_metric(vl_css_layout_node_t *node, const char *property, vl_css_size_metric_t metric, float optional_parent_size);
VL_API vl_result_t vl_css_layout_node_deinit(vl_css_layout_node_t *node);

#endif // VELVET_CSS_LAYOUT_H