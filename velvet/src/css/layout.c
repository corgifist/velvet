#include "velvet/css/layout.h"
#include "css/style.h"
#include "css/stylesheet.h"
#include "dom/element.h"
#include "html/tags.h"
#include "support/base_math.h"
#include "support/color.h"
#include "support/da.h"
#include "support/hash.h"
#include "support/result.h"
#include "support/string.h"
#include "web/theme.h"
#include "web/web.h"
#include "support/math.h"
#include <assert.h>

vl_result_t vl_css_layout_node_init(vl_css_layout_node_t *node, const char *tag) {
    if (!node || !tag) return VL_ERROR;
    VL_ZERO_OUT(node);
    node->tag = tag;
    node->affecting_selectors = VL_DA_INIT(vl_css_class_selector_t);
    node->children = VL_DA_INIT(vl_css_layout_node_t*);
    vl_css_style_init(&node->style);
    return VL_SUCCESS;
}

static vl_css_layout_node_t *new_pseudo_element(vl_css_layout_node_t *node, VL_DA(vl_css_class_t*) force_styles) {
    vl_css_layout_node_t *pseudo = &vl_dom_element_new("text")->layout;
    pseudo->web = node->web;
    pseudo->parent = node;
    pseudo->force_styling = force_styles;
    pseudo->is_pseudo = true;
    ((vl_dom_element_t*) pseudo->owner)->owner = ((vl_dom_element_t*) node->owner)->owner;
    return pseudo;
}

static vl_result_t stylesheet_broad_query(vl_css_layout_node_t *node, VL_DA(vl_css_class_t*) *result) {
    vl_css_stylesheet_t *stylesheet = &node->web->stylesheet;
    for (int i = 0; i < VL_DA_LENGTH(stylesheet->classes); i++) {
        const vl_css_class_t *class = stylesheet->classes + i;
        bool match = false;
        for (int j = 0; j < VL_DA_LENGTH(class->selectors); j++) {
            vl_css_class_selector_t *selector = class->selectors + j;
            vl_css_layout_node_t *ancestor = node;
            for (int k = VL_DA_LENGTH(selector->id_chain); k --> 0;) {
                vl_css_class_id_t *id = selector->id_chain + k;
                switch (id->type) {
                case VL_CSS_CLASS_ID_ALL: {
                    match = true;
                    goto next_class;
                    break;
                }
                case VL_CSS_CLASS_ID_ELEMENT: {
                    if (ancestor && ancestor->tag && strcmp(ancestor->tag, id->name) != 0) {
                        goto next_selector;
                    } else {
                        if (ancestor && ancestor->parent) ancestor = ancestor->parent;
                        goto next_id;
                    }
                    break;
                }
                case VL_CSS_CLASS_ID_CLASS: {
                    for (int l = 0; l < VL_DA_LENGTH(ancestor->affecting_selectors); l++) {
                        if (strcmp(ancestor->affecting_selectors[l].id_chain->name, id->name) == 0) {
                            goto next_id;
                        }
                    }
                    goto next_selector;
                    break;
                }
                case VL_CSS_CLASS_ID_UNIQUE_ID: {
                    if (ancestor->unique_id && strcmp(ancestor->unique_id, id->name) == 0) {
                        goto next_id;
                    }
                    goto next_selector;
                    break;
                }
                case VL_CSS_CLASS_ID_PSEUDO_ELEMENT: {
                    // TODO?
                    break;
                }
                }
                next_id:
                continue;
            }
            match = true;
            next_selector:
            if (match) break;
            continue;
        }
        next_class:
        if (match) {
            if (!*result) *result = VL_DA_INIT(vl_css_class_t*);
            VL_DA_APPEND(*result, class);
        }
    }
    return VL_SUCCESS;
}

vl_result_t vl_css_layout_node_refresh_style(vl_css_layout_node_t *node) {
    if (!node) return VL_ERROR;
    vl_css_style_deinit(&node->style);
    vl_css_style_init(&node->style);
    if (node->force_styling) {
        for (int i = 0; i < VL_DA_LENGTH(node->force_styling); i++) {
            vl_css_style_t tmp = {0};
            vl_css_style_from_class(&tmp, node->force_styling[i]);
            vl_css_style_merge(&node->style, &tmp);
            vl_css_style_deinit(&tmp);
        }
    }

    VL_DA(vl_css_class_t*) matched_classes = NULL;
    // vl_css_stylesheet_broad_query(node->stylesheet, &node->tag_selector, &matched_classes);
    // vl_css_stylesheet_broad_query(node->stylesheet, &node->unique_id_selector, &matched_classes);
    // if (node->affecting_selectors) {
    //     for (int i = 0; i < VL_DA_LENGTH(node->affecting_selectors); i++) {
    //         vl_css_stylesheet_broad_query(node->stylesheet, node->affecting_selectors + i, &matched_classes);
    //     }
    // }
    stylesheet_broad_query(node, &matched_classes);
    VL_DA(vl_css_class_t*) matched_before_classes = NULL;
    VL_DA(vl_css_class_t*) matched_after_classes = NULL;
    if (matched_classes) {
        for (int i = 0; i < VL_DA_LENGTH(matched_classes); i++) {
            vl_css_class_t *matched_class = matched_classes[i];
            vl_css_style_t tmp_style = {0};
            bool is_before = false;
            bool is_after = false;
            for (int j = 0; j < VL_DA_LENGTH(matched_class->selectors); j++) {
                vl_css_class_selector_t *selector = matched_class->selectors + j;
                for (int k = 0; k < VL_DA_LENGTH(selector->id_chain); k++) {
                    vl_css_class_id_t *id = selector->id_chain + k;
                    if (id->type == VL_CSS_CLASS_ID_PSEUDO_ELEMENT && strcmp(id->name, "before") == 0) {
                        is_before = true;
                    }
                    if (id->type == VL_CSS_CLASS_ID_PSEUDO_ELEMENT && strcmp(id->name, "after") == 0) {
                        is_after = true;
                    }
                }
            }
            vl_css_style_from_class(&tmp_style, matched_class);
            if (is_before || is_after) {
                VL_DA(vl_css_class_t*) *pseudo_classes = (is_before ? &matched_before_classes : &matched_after_classes);
                if (!*pseudo_classes) *pseudo_classes = VL_DA_INIT(vl_css_class_t*);
                VL_DA_APPEND(*pseudo_classes, matched_class);
                goto next;
            }
            vl_css_style_merge(&node->style, &tmp_style);
            next:
            vl_css_style_deinit(&tmp_style);
        }
    }
    VL_DA_FREE(matched_classes);
    if (node->is_pseudo) {
        vl_css_value_t content_string = vl_css_style_get_property(&node->style, "content", VL_CSS_VALUE_NONE());
        if (!VL_CSS_VALUE_IS_LITERAL(content_string)) {
            node->content_hash = 0;
        } else {
            vl_dom_element_set_string(node->owner, "innerText", content_string.as.literal);
            node->content_hash = vl_hash_string(content_string.as.literal);
        }
    }
    vl_css_style_merge_inline(&node->style, &node->inline_style);
    vl_css_style_print(&node->style);
    if (matched_before_classes) {
        if (!node->pseudo_before) {
            node->pseudo_before = new_pseudo_element(node, matched_before_classes);
        }
    }
    if (matched_after_classes) {
        if (!node->pseudo_after) {
            node->pseudo_after = new_pseudo_element(node, matched_after_classes);
        }
    }
    return VL_SUCCESS;
}

#include "bits/display_block.c"

static vl_result_t layout_center(vl_css_layout_node_t *node) {
    VL_DA(vl_css_block_line) lines = layout_generic_div_ex(node);
    for (int i = 0; i < VL_DA_LENGTH(lines); i++) {
        vl_css_block_line *line = lines + i;
        for (int j = 0; j < VL_DA_LENGTH(line->elements); j++) {
            vl_css_layout_node_t *element = line->elements[j];
            if (element->tag && strcmp(element->tag, "text") != 0)
                element->position.x += node->size.x / 2 - line->width / 2;
        }
    }
    for (int i = 0; i < VL_DA_LENGTH(lines); i++) {
        VL_DA_FREE(lines[i].elements);
    }
    VL_DA_FREE(lines);
    return VL_SUCCESS;
}

static vl_result_t layout_html(vl_css_layout_node_t *node) {
    layout_generic_div(node);
    float y_offset = 0;
    for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
        vl_css_layout_node_t *child = node->children[i];
        y_offset = VL_MAX(y_offset, child->position.y + child->size.y + VL_MAX(child->margin.z, child->block_last_margin) + node->effective_padding.z);
    }
    node->size.y = VL_MAX(node->size.y, y_offset);
    return VL_SUCCESS;
}

static vl_result_t layout_body(vl_css_layout_node_t *node) {
    layout_generic_div(node);
    if (node->web->dom.quirks) return VL_SUCCESS;
    if (node->block_first_offset - node->margin.x < 0) {
        node->position.y += node->margin.x - node->block_first_offset;
    }
    return VL_SUCCESS;
}

typedef vl_result_t (*layout_func)(vl_css_layout_node_t *node);
static const struct {
    const char *tag;
    layout_func layout;
} s_layout_overrides[] = {
    {"html", layout_html}, 
    {"body", layout_body},
    {"p", layout_generic_div},
    {"div", layout_generic_div},
    {"span", layout_generic_div},
    {"center", layout_center},
    {"h1", layout_generic_div},
    {"h2", layout_generic_div},
    {"h3", layout_generic_div},
    {"h4", layout_generic_div},
    {"h5", layout_generic_div},    
    {"h6", layout_generic_div},
    {"code", layout_generic_div},
    {"hr", layout_generic_div},
    {"article", layout_generic_div},
    {"strong", layout_generic_div}
};

#include "bits/generic_metric_to_metric4.c"
#include "bits/construct_complex_metric.c"

#include "bits/construct_dimensions.c"
#include "bits/construct_borders.c"
#include "bits/construct_background.c"

#include "bits/construct_position_metrics.c"

static vl_css_layout_display_t get_display_mode(vl_css_layout_node_t *node) {
    if (!node) return VL_CSS_LAYOUT_DISPLAY_NONE;
    bool is_inline = vl_html_is_tag_inline(node->tag);
    vl_css_value_t display_value = vl_css_layout_node_get_property(node, "display", VL_CSS_VALUE_CONST_LITERAL(is_inline ? "inline" : "block"));
    if (VL_CSS_VALUE_COMPARE_LITERALS(display_value, "none")) return VL_CSS_LAYOUT_DISPLAY_NONE;
    if (VL_CSS_VALUE_COMPARE_LITERALS(display_value, "block")) return VL_CSS_LAYOUT_DISPLAY_BLOCK;
    if (VL_CSS_VALUE_COMPARE_LITERALS(display_value, "inline")) return VL_CSS_LAYOUT_DISPLAY_INLINE;
    return VL_CSS_LAYOUT_DISPLAY_BLOCK;
}

static vl_css_layout_position_type_t get_position_type(vl_css_value_t value) {
    if (!VL_CSS_VALUE_IS_LITERAL(value)) return VL_CSS_LAYOUT_POSITION_STATIC;
    if (VL_CSS_VALUE_COMPARE_LITERALS(value, "static")) return VL_CSS_LAYOUT_POSITION_STATIC;
    if (VL_CSS_VALUE_COMPARE_LITERALS(value, "absolute")) return VL_CSS_LAYOUT_POSITION_ABSOLUTE;
    if (VL_CSS_VALUE_COMPARE_LITERALS(value, "relative")) return VL_CSS_LAYOUT_POSITION_RELATIVE;
    return VL_CSS_LAYOUT_POSITION_STATIC;
}

vl_result_t vl_css_layout_node_process(vl_css_layout_node_t *node) {
    if (!node) return VL_ERROR;
    if (node->calculating_layout) return VL_SUCCESS;
    if (node->tag && strcmp(node->tag, "root") == 0) {
        return VL_SUCCESS;
    }
    vl_css_layout_node_refresh_style(node);
    node->calculating_layout = true;
    node->position = VL_VEC2(0);
    node->display = get_display_mode(node);
    node->block_last_margin = 0;
    node->block_applied_margin = VL_FLOAT_MIN;
    node->block_first_margin = VL_FLOAT_MIN;
    node->span_y_offset = 0;
    vl_color_t default_color = (node->parent ? node->parent->color : VL_BLACK);
    if (strcmp(node->tag, "text") == 0) {
        default_color = vl_web_theme_get_property(node->web->theme, "canvastext", default_color);
    }
    vl_css_value_t css_color = vl_css_layout_node_get_property(node, "color", VL_CSS_VALUE_RGBA(default_color.r, default_color.g, default_color.b, default_color.a));
    if (!VL_CSS_VALUE_COLOR_COMPATIBLE(css_color)) {
        css_color = VL_CSS_VALUE_RGBA(0, 0, 0, 1);
    }
    node->color = vl_css_value_to_rgba(css_color);
    node->position_type = get_position_type(vl_css_layout_node_get_property(node, "position", VL_CSS_VALUE_CONST_LITERAL("static")));
    node->position_metrics = construct_position_metrics(node);
    construct_dimensions(node);
    construct_complex_metric("margin", &node->margin, node->auto_margin, node);
    construct_complex_metric("padding", &node->padding, NULL, node);
    construct_borders(node);
    construct_background(node);
    node->effective_padding = VL_VEC4_ADD(node->padding, VL_VEC4(node->border[0].width, node->border[1].width, node->border[2].width, node->border[3].width));
    for (int i = 0; i < VL_ARR_LEN(s_layout_overrides); i++) {
        if (strcmp(node->tag, s_layout_overrides[i].tag) == 0) {
            s_layout_overrides[i].layout(node);
            goto final;
        }
    }
    node->size = vl_css_layout_node_get_raw_content_size(node);

    final:
    if (node->auto_margin[1] && node->auto_margin[3]) {
        node->margin.y = (node->parent->size.x / 2 - node->size.x / 2);
        node->margin.w = node->margin.y;
    } else if (node->auto_margin[3]) {
        node->margin.w = node->parent->size.x - node->size.x;
    } else if (node->auto_margin[1]) {
        node->margin.y = node->parent->size.x - node->size.x;
    }
    node->position.x += node->margin.w;
    node->position.y += node->margin.x;
    node->calculating_layout = false;
    return VL_SUCCESS;
}

vl_vec2_t vl_css_layout_node_get_raw_content_size(vl_css_layout_node_t *node) {
    if (!node || !node->get_content_size) return node->size;
    return node->get_content_size(node);
}

static const char *s_inherited_properties[] = {
    "color",
    "--velvet-element-highlight",
    "--velvet-margin-highlight",
    "font-family",
    "text-align"
};

vl_css_value_t vl_css_layout_node_get_property(vl_css_layout_node_t *node, const char *property, vl_css_value_t fallback) {
    if (!node || !property) return fallback;
    vl_css_value_t result = fallback;
    if (node->style.applied_rules) {
        result = vl_css_style_get_property(&node->style, property, VL_CSS_VALUE_NONE());
    }
    bool property_automatically_inherited = false;
    for (int i = 0; i < VL_ARR_LEN(s_inherited_properties); i++) {
        if (strcmp(property, s_inherited_properties[i]) == 0) {
            property_automatically_inherited = true;
        }
    }
    if ((result.type == VL_CSS_VALUE_NONE && property_automatically_inherited) || VL_CSS_VALUE_COMPARE_LITERALS(result, "inherit")) {
        result = vl_css_layout_node_get_property(node->parent, property, fallback);
    }
    if (VL_CSS_VALUE_COMPARE_LITERALS(result, "unset") || VL_CSS_VALUE_COMPARE_LITERALS(result, "initial")) {
        result = fallback;
    }
    if (result.type == VL_CSS_VALUE_CONST_LITERAL 
            && vl_web_theme_supports_property(result.as.literal)) {
        vl_color_t theme_color = vl_web_theme_get_property(node->web->theme, result.as.literal, VL_COLOR(0));
        result = VL_CSS_VALUE_RGBA(theme_color.r, theme_color.g, theme_color.b, theme_color.a);
    }
    if (result.type == VL_CSS_VALUE_NONE) {
        result = fallback;
    }
    return result;
}

vl_css_size_metric_t vl_css_layout_node_process_metric(vl_css_layout_node_t *node, const char *property, vl_css_size_metric_t metric, float optional_parent_size) {
    float parent_size = optional_parent_size;
    if (metric.type == VL_CSS_SIZE_METRIC_PIXELS) return metric;
    if (metric.type == VL_CSS_SIZE_METRIC_PERCENTAGE) return VL_CSS_SIZE_PIXELS(parent_size * metric.value);
    if (metric.type == VL_CSS_SIZE_METRIC_EM) {
        bool is_font_size = (property && strcmp(property, "font-size") == 0);
        vl_css_value_t font_size = vl_css_layout_node_get_property(is_font_size ? node->parent : node, "font-size", VL_CSS_VALUE_METRIC1(VL_CSS_SIZE_EM(1)));
        vl_css_size_metric_t processed_metric = font_size.as.metric1;
        float base_scale = 16;
        if (is_font_size) {
            vl_css_value_t font_family = vl_css_layout_node_get_property(node, "font-family", VL_CSS_VALUE_NONE());
            if (VL_CSS_VALUE_COMPARE_LITERALS(font_family, "monospace")) {
                base_scale = 13;
            }
        }
        if (node->tag && strcmp(node->tag, "html") != 0) {
            processed_metric = vl_css_layout_node_process_metric(is_font_size ? node->parent : node, "font-size", processed_metric, base_scale);
        } else {
            processed_metric = VL_CSS_SIZE_PIXELS(processed_metric.type == VL_CSS_SIZE_METRIC_PIXELS
                                                    ? processed_metric.value
                                                    : parent_size * processed_metric.value);
        }
        return VL_CSS_SIZE_PIXELS(processed_metric.value * metric.value);
    }
    if (metric.type == VL_CSS_SIZE_METRIC_REM) {
        vl_css_value_t font_size = vl_css_layout_node_get_property(&node->web->dom.root->layout, "font-size", VL_CSS_VALUE_METRIC1(VL_CSS_SIZE_PIXELS(16)));
        vl_css_size_metric_t processed_metric = font_size.as.metric1;
        if (property && strcmp(node->tag, "html") != 0 && strcmp(property, "font-size") != 0) {
            processed_metric = vl_css_layout_node_process_metric(&node->web->dom.root->layout, "font-size", processed_metric, 0);
        } else {
            processed_metric = VL_CSS_SIZE_PIXELS(processed_metric.type == VL_CSS_SIZE_METRIC_PIXELS
                                                    ? processed_metric.value
                                                    : 16 * processed_metric.value);
        }
        return VL_CSS_SIZE_PIXELS(processed_metric.value * metric.value);
    }
    return metric;
}

vl_result_t vl_css_layout_node_deinit(vl_css_layout_node_t *node) {
    if (!node) return VL_ERROR;
    VL_STRING_FREE(node->unique_id);
    if (node->affecting_selectors) {
        for (int i = 0; i < VL_DA_LENGTH(node->affecting_selectors); i++) {
            vl_css_class_selector_deinit(node->affecting_selectors + i);
        }
        VL_DA_FREE(node->affecting_selectors);
    }
    if (node->force_styling) {
        VL_DA_FREE(node->force_styling);
    }
    vl_css_style_deinit(&node->style);
    vl_css_inline_style_deinit(&node->inline_style);
    if (node->pseudo_before) {
        vl_dom_element_free(node->pseudo_before->owner);
    }
    if (node->pseudo_after) {
        vl_dom_element_free(node->pseudo_after->owner);
    }
    if (node->children) {
        for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
            vl_css_layout_node_deinit(node->children[i]);
        }
        VL_DA_FREE(node->children);
    }
    VL_ZERO_OUT(node);
    return VL_SUCCESS;
}