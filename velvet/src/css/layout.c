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
    node->class_names = NULL;
    node->children = VL_DA_INIT(vl_css_layout_node_t*, 1);
    vl_css_style_init(&node->style);
    return VL_SUCCESS;
}

static const char *s_pseudo_to_string[] = {
    [VL_CSS_LAYOUT_PSEUDO_ELEMENT_NONE] = "none",
    [VL_CSS_LAYOUT_PSEUDO_ELEMENT_BEFORE] = "before",
    [VL_CSS_LAYOUT_PSEUDO_ELEMENT_AFTER] = "after"
};

static vl_css_layout_node_t *new_pseudo_element(vl_css_layout_node_t *node, vl_css_layout_pseudo_element_type_t type) {
    vl_css_layout_node_t *pseudo = &vl_dom_element_new("text")->layout;
    pseudo->web = node->web;
    pseudo->parent = node;
    pseudo->pseudo_type = type;
    ((vl_dom_element_t*) pseudo->owner)->owner = ((vl_dom_element_t*) node->owner)->owner;
    ((vl_dom_element_t*) pseudo->owner)->parent = ((vl_dom_element_t*) node->owner);
    return pseudo;
}

static vl_result_t stylesheet_broad_query(vl_css_layout_node_t *node, VL_DA(vl_css_class_t*) *result) {
    vl_css_stylesheet_t *stylesheet = &node->web->stylesheet;
    for (int i = 0; i < VL_DA_LENGTH(stylesheet->classes); i++) {
        const vl_css_class_t *class = stylesheet->classes + i;
        bool match = false;
        for (int j = 0; j < VL_DA_LENGTH(class->selectors); j++) {
            vl_css_class_selector_t *selector = class->selectors + j;
            vl_css_layout_node_t *level = node;
            for (int k = VL_DA_LENGTH(selector->hierarchy); k --> 0;) {
                vl_css_class_id_t *id = selector->hierarchy + k;
                vl_css_layout_node_t *target_level = (level->pseudo_type ? level->parent : level);  
                bool has_before = false;
                bool has_after = false;
                bool gate_pseudo_elements = false;
                for (int l = VL_DA_LENGTH(id->atoms); l --> 0;) {
                    vl_css_class_atom_t *atom = id->atoms + l;
                    switch (atom->type) {
                    case VL_CSS_CLASS_ATOM_ALL: {
                        match = true;
                        goto next_selector;
                    }
                    case VL_CSS_CLASS_ATOM_ELEMENT: {
                        if (level->pseudo_type && !gate_pseudo_elements) goto next_selector;
                        if (atom->as.string && strcmp(target_level->tag, atom->as.string) != 0) {
                            goto next_selector;
                        }
                        break;
                    }
                    case VL_CSS_CLASS_ATOM_UNIQUE_ID: {
                        if (level->pseudo_type && !gate_pseudo_elements) goto next_selector;
                        if (atom->as.string && level->unique_id && strcmp(level->unique_id, atom->as.string) != 0) {
                            goto next_selector;
                        }
                        break;
                    }
                    case VL_CSS_CLASS_ATOM_CLASS_NAMES: {
                        if (level->pseudo_type && !gate_pseudo_elements) goto next_selector;
                        if (atom->as.class_names && target_level->class_names) {
                            int len = VL_DA_LENGTH(atom->as.class_names);
                            for (int i1 = 0; i1 < len; i1++) {
                                bool found = false;
                                for (int j1 = 0; j1 < VL_DA_LENGTH(target_level->class_names); j1++) {
                                    if (strcmp(atom->as.class_names[i1], target_level->class_names[j1]) == 0) {
                                        found = true;
                                        break;
                                    }
                                }
                                if (!found) goto next_selector;
                            }
                        } else {
                            goto next_selector;
                        }
                        break;
                    }
                    case VL_CSS_CLASS_ATOM_PSEUDO_ELEMENT: {
                        vl_css_layout_pseudo_element_type_t pseudo_type = 0;
                        for (int i1 = 0; i1 < VL_ARR_LEN(s_pseudo_to_string); i1++) {
                            if (atom->as.string && strcmp(s_pseudo_to_string[i1], atom->as.string) == 0) {
                                pseudo_type = i1;
                                break;
                            }
                        }
                        if (pseudo_type == 0) goto next_selector;
                        if (level->pseudo_type == 0) {
                            if (pseudo_type == VL_CSS_LAYOUT_PSEUDO_ELEMENT_BEFORE) {
                                has_before = true;
                            }
                            if (pseudo_type == VL_CSS_LAYOUT_PSEUDO_ELEMENT_AFTER) {
                                has_after = true;
                            }
                        } else if (level->pseudo_type != pseudo_type) {
                            goto next_selector;
                        }
                        gate_pseudo_elements = true;
                        break;
                    }
                    default: break;
                    }
                }
                if (level->parent) {
                    if (has_before && !level->pseudo_before) {
                        level->pseudo_before = new_pseudo_element(level, VL_CSS_LAYOUT_PSEUDO_ELEMENT_BEFORE);
                        goto next_selector;
                    }
                    if (has_after && !level->pseudo_after) {
                        level->pseudo_after = new_pseudo_element(level, VL_CSS_LAYOUT_PSEUDO_ELEMENT_AFTER);
                        goto next_selector;
                    }
                    if (gate_pseudo_elements && !level->pseudo_type) goto next_selector;
                    level = level->parent;
                } else goto next_selector;
            }
            match = true;
            next_selector:
            if (match) goto next_class;
        }
        next_class:
        if (match) {
            if (!*result) *result = VL_DA_INIT(vl_css_class_t*);
            VL_DA_APPEND(*result, class);
        }
    }
    return VL_SUCCESS;
}

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

vl_result_t vl_css_layout_node_refresh_style(vl_css_layout_node_t *node) {
    if (!node) return VL_ERROR;
    vl_css_style_deinit(&node->style);
    vl_css_style_init(&node->style);

    VL_DA(vl_css_class_t*) matched_classes = NULL;
    stylesheet_broad_query(node, &matched_classes);

    if (matched_classes) {
        for (int i = 0; i < VL_DA_LENGTH(matched_classes); i++) {
            vl_css_class_t *matched_class = matched_classes[i];
            vl_css_style_t tmp_style = {0};
            vl_css_style_from_class(&tmp_style, matched_class);
            vl_css_style_merge(&node->style, &tmp_style);
            vl_css_style_deinit(&tmp_style);
        }
    }
    VL_DA_FREE(matched_classes);
    vl_css_style_merge_inline(&node->style, &node->inline_style);
    // vl_css_style_print(&node->style);

    node->display = get_display_mode(node);    
    vl_color_t default_color = (node->parent ? node->parent->color : VL_BLACK);
    if (strcmp(node->tag, "text") == 0) {
        default_color = vl_web_theme_get_property(node->web->theme, "canvastext", default_color);
    }
    vl_css_value_t css_color = vl_css_layout_node_get_property(node, "color", VL_CSS_VALUE_RGBA(default_color.r, default_color.g, default_color.b, default_color.a));
    if (!VL_CSS_VALUE_COLOR_COMPATIBLE(css_color)) {
        css_color = VL_CSS_VALUE_RGBA(0, 0, 0, 1);
    }
    node->color = vl_css_value_to_rgba(css_color);
    vl_css_value_t content_value = vl_css_layout_node_get_property(node, "content", VL_CSS_VALUE_NONE());
    if (VL_CSS_VALUE_IS_LITERAL(content_value)) {
        VL_STRING_FREE(node->content_string);
        node->content_string = VL_STRING_INIT(content_value.as.literal);
    } else {
        node->content_string = NULL;
    }
    node->position_type = get_position_type(vl_css_layout_node_get_property(node, "position", VL_CSS_VALUE_CONST_LITERAL("static")));
    node->position_metrics = construct_position_metrics(node);
    construct_dimensions(node);
    construct_complex_metric("margin", &node->margin, node->auto_margin, node);
    construct_complex_metric("padding", &node->padding, NULL, node);
    construct_borders(node);
    construct_background(node);
    node->effective_padding = VL_VEC4_ADD(node->padding, VL_VEC4(node->border[0].width, node->border[1].width, node->border[2].width, node->border[3].width));
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


vl_result_t vl_css_layout_node_process(vl_css_layout_node_t *node) {
    if (!node) return VL_ERROR;
    vl_css_layout_node_refresh_style(node);
    return vl_css_layout_node_layout(node);
}

vl_result_t vl_css_layout_node_layout(vl_css_layout_node_t *node) {
    if (!node) return VL_ERROR;
    if (node->calculating_layout) return VL_SUCCESS;
    if (node->tag && strcmp(node->tag, "root") == 0) {
        return VL_SUCCESS;
    }
    // node->position = VL_VEC2();
    node->block_last_margin = 0;
    node->block_applied_margin = VL_FLOAT_MIN;
    node->block_first_margin = VL_FLOAT_MIN;
    node->span_y_offset = 0;
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
    if (node->class_names) {
        for (int i = 0; i < VL_DA_LENGTH(node->class_names); i++) {
            VL_STRING_FREE(node->class_names[i]);
        }
        VL_DA_FREE(node->class_names);
    }
    VL_STRING_FREE(node->content_string);
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