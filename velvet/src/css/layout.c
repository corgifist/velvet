#include "velvet/css/layout.h"
#include "css/style.h"
#include "html/tags.h"
#include "support/da.h"
#include "support/result.h"
#include "web/web.h"
#include "support/math.h"
#include <stdlib.h>

vl_result_t vl_css_layout_node_init(vl_css_layout_node_t *node, const char *tag) {
    if (!node || !tag) return VL_ERROR;
    VL_ZERO_OUT(node);
    node->tag = tag;
    if (strcmp(tag, "text") != 0) {
        node->tag_selector.id_chain = VL_DA_INIT(vl_css_class_id_t);
        *VL_DA_PUSH(node->tag_selector.id_chain, vl_css_class_id_t) = (vl_css_class_id_t) {
            .type = VL_CSS_CLASS_ID_ELEMENT,
            .name = VL_DA_INIT_FROM_STRING(tag)
        };
    }
    node->affecting_selectors = VL_DA_INIT(vl_css_class_selector_t);
    node->children = VL_DA_INIT(vl_css_layout_node_t*);
    vl_css_style_init(&node->style);
    return VL_SUCCESS;
}

vl_result_t vl_css_layout_node_refresh_style(vl_css_layout_node_t *node) {
    if (!node) return VL_ERROR;
    vl_css_style_deinit(&node->style);
    vl_css_style_init(&node->style);

    VL_DA(vl_css_class_t*) matched_classes = NULL;
    vl_css_stylesheet_broad_query(node->stylesheet, &node->tag_selector, &matched_classes);
    if (node->affecting_selectors) {
        for (int i = 0; i < VL_DA_LENGTH(node->affecting_selectors); i++) {
            vl_css_stylesheet_broad_query(node->stylesheet, node->affecting_selectors + i, &matched_classes);
        }
    }
    if (matched_classes) {
        for (int i = 0; i < VL_DA_LENGTH(matched_classes); i++) {
            vl_css_style_t tmp_style = {0};
            vl_css_style_from_class(&tmp_style, matched_classes[i]);
            vl_css_style_merge(&node->style, &tmp_style);
            vl_css_style_deinit(&tmp_style);
        }
    }
    VL_DA_FREE(matched_classes);
    // vl_css_style_print(&node->style);
    return VL_SUCCESS;
}


static vl_vec4_t generic_metric_to_metric4(vl_css_layout_node_t *node, vl_css_value_t value) {
    if (value.type == VL_CSS_VALUE_SIZE_METRIC1) {
        float mx = vl_css_layout_node_process_metric(node, NULL, value.as.metric1, node->parent->size.x).value;
        float my = vl_css_layout_node_process_metric(node, NULL, value.as.metric1, node->parent->size.y).value;
        return (vl_vec4_t) {my, mx, my, mx};
    }
    if (value.type == VL_CSS_VALUE_SIZE_METRIC2) {
        float mx = vl_css_layout_node_process_metric(node, NULL, value.as.metric2[1], node->parent->size.x).value;
        float my = vl_css_layout_node_process_metric(node, NULL, value.as.metric2[0], node->parent->size.y).value;
        return VL_VEC4(my, mx, my, mx);
    }
    if (value.type == VL_CSS_VALUE_SIZE_METRIC3) {
        float mt = vl_css_layout_node_process_metric(node, NULL, value.as.metric3[0], node->parent->size.y).value;
        float mb = vl_css_layout_node_process_metric(node, NULL, value.as.metric3[2], node->parent->size.y).value;
        float mx = vl_css_layout_node_process_metric(node, NULL, value.as.metric3[1], node->parent->size.x).value;
        return VL_VEC4(mt, mx, mb, mx);
    }
    if (value.type == VL_CSS_VALUE_SIZE_METRIC4) {
        float x = vl_css_layout_node_process_metric(node, NULL, value.as.metric4[0], node->parent->size.y).value;
        float y = vl_css_layout_node_process_metric(node, NULL, value.as.metric4[1], node->parent->size.x).value;
        float z = vl_css_layout_node_process_metric(node, NULL, value.as.metric4[2], node->parent->size.y).value;
        float w = vl_css_layout_node_process_metric(node, NULL, value.as.metric4[3], node->parent->size.x).value;
        return VL_VEC4(x, y, z, w);
    }
    return (vl_vec4_t) {0};
}

static vl_css_size_metric_t select_metric(vl_css_rule_t *rule, int i1, int i2, int i3) {
    switch (rule->value.type) {
    case VL_CSS_VALUE_SIZE_METRIC1: return rule->value.as.metric1;
    case VL_CSS_VALUE_SIZE_METRIC2: return rule->value.as.metric2[i1];
    case VL_CSS_VALUE_SIZE_METRIC3: return rule->value.as.metric3[i2];
    case VL_CSS_VALUE_SIZE_METRIC4: return rule->value.as.metric4[i3];
    default: return VL_CSS_SIZE_METRIC(0, 0);
    }
}

static vl_vec4_t construct_margin(vl_css_layout_node_t *node) {
    vl_vec4_t margin = {0};
    for (int i = 0; i < VL_DA_LENGTH(node->style.applied_rules); i++) {
        vl_css_rule_t *rule = node->style.applied_rules[i];
        if (!rule->property) continue;
        if (strcmp(rule->property, "margin") == 0) {
            margin = generic_metric_to_metric4(node->parent, rule->value);
            continue;
        }
        if (strcmp(rule->property, "margin-top") == 0) {
            vl_css_size_metric_t metric = select_metric(rule, 0, 0, 0);
            margin.x = vl_css_layout_node_process_metric(node, NULL, metric, node->parent->size.y).value;
            continue;
        }
        if (strcmp(rule->property, "margin-right") == 0) {
            vl_css_size_metric_t metric = select_metric(rule, 1, 1, 1);
            margin.y = vl_css_layout_node_process_metric(node, NULL, metric, node->parent->size.x).value;
            continue;
        }
        if (strcmp(rule->property, "margin-bottom") == 0) {
            vl_css_size_metric_t metric = select_metric(rule, 0, 2, 2);
            margin.z = vl_css_layout_node_process_metric(node, NULL, metric, node->parent->size.y).value;
            continue;
        }
        if (strcmp(rule->property, "margin-left") == 0) {
            vl_css_size_metric_t metric = select_metric(rule, 1, 1, 3);
            margin.w = vl_css_layout_node_process_metric(node, NULL, metric, node->parent->size.x).value;
            continue;
        }
        if (strcmp(rule->property, "margin-block-start") == 0) {
            margin.x = vl_css_layout_node_process_metric(node, NULL, rule->value.as.metric1, node->parent->size.y).value;
            continue;
        }
        if (strcmp(rule->property, "margin-block-end") == 0) {
            margin.z = vl_css_layout_node_process_metric(node, NULL, rule->value.as.metric1, node->parent->size.y).value;
            continue;
        }
    }
    // printf("%s margin: %f %f %f %f\n", node->tag, margin.x, margin.y, margin.z, margin.w);
    return margin;
}

typedef vl_result_t (*layout_func)(vl_css_layout_node_t *node);

static vl_result_t layout_html(vl_css_layout_node_t *node) {
    for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
        vl_css_layout_node_t *child = node->children[i];
        if (strcmp(child->tag, "body") == 0) {
            node->position = VL_VEC2(0, 0);
            node->size.x = node->parent->size.x;
            vl_css_layout_node_process(child);
            node->size = child->size;
            node->size.y += child->position.y;
            node->size.x += child->margin.y + child->margin.w;
            // node->size.y += child->margin.z;
        }
    }
    return VL_SUCCESS;
}

static vl_css_value_t get_display_mode(vl_css_layout_node_t *node) {
    if (!node) return VL_CSS_VALUE_CONST_LITERAL(NULL);
    bool is_inline = vl_html_is_tag_inline(node->tag);
    return vl_css_layout_node_get_property(node, "display", VL_CSS_VALUE_CONST_LITERAL(is_inline ? "inline" : "block"));
}

static vl_result_t layout_generic_div(vl_css_layout_node_t *node) {
    vl_css_value_t node_display = get_display_mode(node);
    if (VL_CSS_CONST_LITERAL_EQUAL(node_display, "block")) 
        node->size.x = node->parent->size.x - node->margin.y - node->margin.w;
    vl_vec2_t cursor = {0, 0};
    vl_vec2_t size = {node->size.x, node->size.y};
    VL_DA(vl_css_layout_node_t*) layout_targets = VL_DA_INIT(vl_css_layout_node_t*);
    for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
        vl_css_layout_node_t *child = node->children[i];
        vl_css_layout_node_process(child);
        vl_css_value_t display_value = get_display_mode(child);
        if (VL_CSS_CONST_LITERAL_EQUAL(display_value, "none")) continue;
        VL_DA_APPEND(layout_targets, child);
    }
    int len = VL_DA_LENGTH(layout_targets);
    float line_height = 0;
    for (int i = 0; i < len; i++) {
        vl_css_layout_node_t *prev = i > 0 ? layout_targets[i - 1] : NULL;
        vl_css_layout_node_t *child = layout_targets[i];
        vl_css_layout_node_t *next = i < len - 1 ? layout_targets[i + 1] : NULL;
        node->size.y = cursor.y;
        if (VL_CSS_CONST_LITERAL_EQUAL(child->display, "block") || !child->display.as.const_literal) {
            if (prev) {
                if (VL_CSS_CONST_LITERAL_EQUAL(prev->display, "inline")) {
                    cursor.y += prev->size.y;
                }
                cursor.y += prev->margin.z;
                if (child->margin.x > prev->margin.z) {
                    cursor.y += child->margin.x - prev->margin.z;
                }
            }
            cursor.x = 0;
            child->position.x = cursor.x + child->margin.w;
            child->position.y = cursor.y;
            cursor.y += child->size.y;
            line_height = VL_MAX(line_height, child->size.y);
            size.y = cursor.y;
        } else {
            child->position.x = cursor.x + child->margin.w;
            child->position.y = cursor.y;
            cursor.x += child->size.x;
            line_height = VL_MAX(line_height, child->size.y);
            size.x = VL_MAX(cursor.x, size.x);
            size.y = VL_MAX(size.y, cursor.y + line_height);
            if (cursor.x > node->size.x && next) {
                cursor.x = 0;
                cursor.y += line_height;
                line_height = 0;
            }
        }
    }
    VL_DA_FREE(layout_targets);
    node->size = size;
    return VL_SUCCESS;
}

static vl_result_t layout_body(vl_css_layout_node_t *node) {
    layout_generic_div(node);
    if (node->children) {
        for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
            vl_css_layout_node_t *child = node->children[i];
            vl_css_value_t display = get_display_mode(child);
            if (VL_CSS_CONST_LITERAL_EQUAL(display, "none")) continue;
            vl_vec4_t first_margin = child->margin;
            if (first_margin.x > node->position.y) {
                node->position.y += first_margin.x - node->position.y - node->margin.x;
            }
            break;
        }
    }
    return VL_SUCCESS;
}

static const struct {
    const char *tag;
    layout_func layout;
} s_layout_overrides[] = {
    {"html", layout_html}, 
    {"body", layout_body},
    {"p", layout_generic_div},
    {"div", layout_generic_div},
    {"span", layout_generic_div}
};

vl_result_t vl_css_layout_node_process(vl_css_layout_node_t *node) {
    if (!node) return VL_ERROR;
    if (node->calculating_layout) return VL_SUCCESS;
    if (node->tag && strcmp(node->tag, "root") == 0) {
        return VL_SUCCESS;
    }
    vl_css_layout_node_refresh_style(node);
    node->calculating_layout = true;
    node->display = get_display_mode(node);
    node->margin = construct_margin(node);
    for (int i = 0; i < VL_ARR_LEN(s_layout_overrides); i++) {
        if (strcmp(node->tag, s_layout_overrides[i].tag) == 0) {
            vl_result_t result = s_layout_overrides[i].layout(node);
            goto final;
        }
    }
    node->size = vl_css_layout_node_get_raw_content_size(node);

    final:
    node->position.x += node->margin.w;
    node->position.y += node->margin.x;
    node->calculating_layout = false;
    return VL_SUCCESS;
}

vl_vec2_t vl_css_layout_node_get_raw_content_size(vl_css_layout_node_t *node) {
    if (!node || !node->get_content_size) return VL_VEC2(0, 0);
    return node->get_content_size(node);
}

static const char *s_inherited_properties[] = {
    "color",
    "--velvet-element-highlight"
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
    if ((result.type == VL_CSS_VALUE_NONE && property_automatically_inherited) || VL_CSS_CONST_LITERAL_EQUAL(result, "inherit")) {
        result = vl_css_layout_node_get_property(node->parent, property, fallback);
    }
    if (VL_CSS_CONST_LITERAL_EQUAL(result, "unset") || VL_CSS_CONST_LITERAL_EQUAL(result, "initial")) {
        result = fallback;
    }
    if (result.type == VL_CSS_VALUE_CONST_LITERAL 
            && vl_web_theme_supports_property(result.as.const_literal)) {
        vl_color_t theme_color = vl_web_theme_get_property(node->web->theme, result.as.const_literal, VL_COLOR(0));
        result = VL_CSS_VALUE_RGBA(theme_color.r, theme_color.g, theme_color.b, theme_color.a);
    }
    if (result.type == VL_CSS_VALUE_NONE) {
        result = fallback;
    }
    return result;
}

vl_css_size_metric_t vl_css_layout_node_process_metric(vl_css_layout_node_t *node, const char *property, vl_css_size_metric_t metric, float optional_parent_size) {
    float parent_size = 0;
    if (!property) parent_size = optional_parent_size;
    if (metric.type == VL_CSS_SIZE_METRIC_PIXELS) return metric;
    if (metric.type == VL_CSS_SIZE_METRIC_PERCENTAGE) return VL_CSS_SIZE_PIXELS(parent_size * metric.value);
    if (metric.type == VL_CSS_SIZE_METRIC_EM) {
        bool is_font_size = (property && strcmp(property, "font-size") == 0);
        vl_css_value_t font_size = vl_css_layout_node_get_property(is_font_size ? node->parent : node, "font-size", VL_CSS_VALUE_METRIC1(VL_CSS_SIZE_PIXELS(16)));
        vl_css_size_metric_t em_metric = vl_css_layout_node_process_metric(is_font_size ? node->parent : node, "font-size", font_size.as.metric1, 0);
        return VL_CSS_SIZE_PIXELS(em_metric.value * metric.value);
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
    vl_css_class_selector_deinit(&node->tag_selector);
    vl_css_class_selector_deinit(&node->tag_selector);
    if (node->affecting_selectors) {
        for (int i = 0; i < VL_DA_LENGTH(node->affecting_selectors); i++) {
            vl_css_class_selector_deinit(node->affecting_selectors + i);
        }
        VL_DA_FREE(node->affecting_selectors);
    }
    vl_css_style_deinit(&node->style);
    if (node->children) {
        for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
            vl_css_layout_node_deinit(node->children[i]);
        }
        VL_DA_FREE(node->children);
    }
    VL_ZERO_OUT(node);
    return VL_SUCCESS;
}