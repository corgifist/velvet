#include "velvet/css/layout.h"
#include "css/style.h"
#include "support/da.h"
#include "support/result.h"
#include "web/web.h"
#include "support/math.h"

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

static const char *s_inherited_properties[] = {
    "color",
    "--velvet-element-highlight",
    "font-size"
};

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
    vl_css_style_print(&node->style);
    return VL_SUCCESS;
}

static float process_metric(vl_css_layout_node_t *node, float parent_size, vl_css_size_metric_t metric) {
    if (metric.type == VL_CSS_SIZE_METRIC_PERCENTAGE) return parent_size * metric.value;
    if (metric.type == VL_CSS_SIZE_METRIC_EM) {
        vl_css_value_t font_size = vl_css_layout_node_get_property(node, "font-size", VL_CSS_VALUE_METRIC1(VL_CSS_SIZE_PIXELS(16)));
        return font_size.as.metric1.value * metric.value;
    }
    return metric.value;
}

static vl_vec4_t generic_metric_to_metric4(vl_css_layout_node_t *node, vl_css_value_t value) {
    if (value.type == VL_CSS_VALUE_SIZE_METRIC1) {
        float mx = process_metric(node, node->parent->size.x, value.as.metric1);
        float my = process_metric(node, node->parent->size.y, value.as.metric1);
        return (vl_vec4_t) {my, mx, my, mx};
    }
    return (vl_vec4_t) {0};
}

static vl_css_size_metric_t generic_metric_to_metric1(vl_css_value_t value) {
    if (value.type == VL_CSS_VALUE_SIZE_METRIC1) return value.as.metric1;
    return VL_CSS_SIZE_PIXELS(0);
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
            margin.x = process_metric(node, node->parent->size.y, generic_metric_to_metric1(rule->value));
            continue;
        }
        if (strcmp(rule->property, "margin-right") == 0) {
            margin.y = process_metric(node, node->parent->size.x, generic_metric_to_metric1(rule->value));
            continue;
        }
        if (strcmp(rule->property, "margin-bottom") == 0) {
            margin.z = process_metric(node, node->parent->size.y, generic_metric_to_metric1(rule->value));
            continue;
        }
        if (strcmp(rule->property, "margin-left") == 0) {
            margin.w = process_metric(node, node->parent->size.x, generic_metric_to_metric1(rule->value));
            continue;
        }
    }
    vl_css_value_t margin_block_start = vl_css_layout_node_get_property(node, "margin-block-start", VL_CSS_VALUE_METRIC1(VL_CSS_SIZE_PIXELS(16)));
    vl_css_value_t margin_block_end = vl_css_layout_node_get_property(node, "margin-block-end", VL_CSS_VALUE_METRIC1(VL_CSS_SIZE_PIXELS(16)));
    vl_vec2_t margin_block = {
        margin_block_start.as.metric1.value,
        margin_block_end.as.metric1.value
    };
    margin.x += margin_block.x;
    margin.z += margin_block.y;
    printf("margin_block: %f %f\n", margin_block.x, margin_block.y);
    return margin;
}

typedef vl_result_t (*layout_func)(vl_css_layout_node_t *node);

static vl_result_t layout_html(vl_css_layout_node_t *node) {
    for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
        vl_css_layout_node_t *child = node->children[i];
        if (strcmp(child->tag, "body") == 0) {
            node->size.x = node->parent->size.x;
            vl_css_layout_node_process(child);
            node->size = child->size;
            node->size.x += child->margin.y + child->margin.w;
            node->size.y += child->margin.x + child->margin.z;
        }
    }
    return VL_SUCCESS;
}

static vl_result_t layout_generic_div(vl_css_layout_node_t *node) {
    vl_vec2_t cursor = {0, 0};
    node->size.x = node->parent->size.x - node->margin.y - node->margin.w;
    vl_vec2_t size = {node->size.x, 0};
    for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
        vl_css_layout_node_t *child = node->children[i];
        node->size.y = cursor.y;
        vl_css_layout_node_process(child);
        child->position.x = cursor.x + child->margin.w;
        child->position.y = cursor.y + child->margin.x;
        cursor.y += child->size.y + child->margin.x + child->margin.z;
        size.y = cursor.y;
    }
    node->size.y = size.y;
    return VL_SUCCESS;
}

static const struct {
    const char *tag;
    layout_func layout;
} s_layout_overrides[] = {
    {"html", layout_html}, 
    {"body", layout_generic_div},
    {"p", layout_generic_div}
};

vl_result_t vl_css_layout_node_process(vl_css_layout_node_t *node) {
    if (!node) return VL_ERROR;
    if (node->calculating_layout) return VL_SUCCESS;
    if (node->tag && strcmp(node->tag, "root") == 0) {
        return VL_SUCCESS;
    }
    vl_css_layout_node_refresh_style(node);
    node->calculating_layout = true;
    node->margin = construct_margin(node);
    printf("margin: %f %f %f %f\n", node->margin.x, node->margin.y, node->margin.z, node->margin.w);
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
    if (strcmp(property, "font-size") == 0 && result.type == VL_CSS_VALUE_SIZE_METRIC1 && result.as.metric1.type == VL_CSS_SIZE_METRIC_EM) {
        vl_css_value_t font_size = vl_css_layout_node_get_property(node->parent, "font-size", VL_CSS_VALUE_METRIC1(VL_CSS_SIZE_PIXELS(16)));
        result = VL_CSS_VALUE_METRIC1(VL_CSS_SIZE_PIXELS(font_size.as.metric1.value * result.as.metric1.value));
    } else if (result.type == VL_CSS_VALUE_SIZE_METRIC1 && result.as.metric1.type == VL_CSS_SIZE_METRIC_EM) {
        vl_css_value_t font_size = vl_css_layout_node_get_property(node, "font-size", VL_CSS_VALUE_METRIC1(VL_CSS_SIZE_PIXELS(16)));
        printf("font size: %f\n", font_size.as.metric1.value);
        result = VL_CSS_VALUE_METRIC1(VL_CSS_SIZE_PIXELS(font_size.as.metric1.value * result.as.metric1.value));
    }
    if (result.type == VL_CSS_VALUE_CONST_LITERAL 
            && vl_web_theme_supports_property(result.as.const_literal)) {
        vl_color_t theme_color = vl_web_theme_get_property(node->web->theme, result.as.const_literal, VL_COLOR(0));
        result = VL_CSS_VALUE_RGBA(theme_color.r, theme_color.g, theme_color.b, theme_color.a);
    }
    return result;
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