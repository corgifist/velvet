#include "velvet/css/layout.h"
#include "velvet/support/str.h"
#include "sides.h"

static vl_css_layout_border_type_t get_border_type(const vl_css_value_t *value);
static float get_border_width(vl_css_layout_node_t *node, const vl_css_value_t *value);
static vl_css_layout_border_t construct_border(vl_css_layout_node_t *node, const vl_css_value_t *value);
static void construct_borders(vl_css_layout_node_t *node);

#ifndef VELVET_CSS_BITS_CONSTRUCT_BORDERS_C
#define VELVET_CSS_BITS_CONSTRUCT_BORDERS_C

static vl_css_layout_border_type_t get_border_type(const vl_css_value_t *value) {
    if (!VL_CSS_VALUE_IS_LITERAL(*value)) return VL_CSS_LAYOUT_BORDER_NONE;
    if (VL_CSS_VALUE_COMPARE_LITERALS(*value, "none")) return VL_CSS_LAYOUT_BORDER_NONE;
    if (VL_CSS_VALUE_COMPARE_LITERALS(*value, "solid")) return VL_CSS_LAYOUT_BORDER_SOLID;
    if (VL_CSS_VALUE_COMPARE_LITERALS(*value, "double")) return VL_CSS_LAYOUT_BORDER_DOUBLE;
    if (VL_CSS_VALUE_COMPARE_LITERALS(*value, "inset")) return VL_CSS_LAYOUT_BORDER_INSET;
    return VL_CSS_LAYOUT_BORDER_NONE;
}

static float get_border_width(vl_css_layout_node_t *node, const vl_css_value_t *value) {
    if (VL_CSS_VALUE_COMPARE_LITERALS(*value, "thin")) return 1;
    if (VL_CSS_VALUE_COMPARE_LITERALS(*value, "medium")) return 3;
    if (VL_CSS_VALUE_COMPARE_LITERALS(*value, "thick")) return 5;
    if (VL_CSS_VALUE_IS_METRIC(*value)) {
        vl_css_size_metric_t processed_metric = vl_css_layout_node_process_metric(node, NULL, value->as.metric1, 0);
        return processed_metric.value;
    }
    return 3;
}

static vl_css_layout_border_t construct_border(vl_css_layout_node_t *node, const vl_css_value_t *value) {
    vl_css_layout_border_t result = {0};
    if (VL_CSS_VALUE_COMPARE_LITERALS(*value, "none")) return result;
    if (value->as.list)
        for (int i = 0; i < VL_DA_LENGTH(value->as.list); i++) {
            vl_css_value_t v = value->as.list[i];
            if (VL_CSS_VALUE_IS_LITERAL(v)) {
                result.type = get_border_type(&v);
                continue;
            }
            if (VL_CSS_VALUE_COLOR_COMPATIBLE(v)) {
                result.color = vl_css_value_to_rgba(v);
                continue;
            }
            if (VL_CSS_VALUE_IS_METRIC(v)) {
                vl_css_size_metric_t processed_metric = vl_css_layout_node_process_metric(node, NULL, v.as.metric1, 0);
                result.width = processed_metric.value;
                continue;
            }
        }
    return result;
}

static void construct_borders(vl_css_layout_node_t *node) {
    VL_ZERO_OUT(node->border, sizeof(node->border));
    vl_css_value_t medium_border = VL_CSS_VALUE_CONST_LITERAL("medium");
    for (int i = 0; i < VL_ARR_LEN(node->border); i++) {
        node->border[i].width = get_border_width(node, &medium_border);
        node->border[i].color = node->color;
    }
    for (int i = 0; i < VL_DA_LENGTH(node->style.applied_rules); i++) {
        const vl_css_rule_t *rule = node->style.applied_rules[i];
        for (int j = 0; j < VL_ARR_LEN(sides); j++) {
            if (strcmp(rule->property, "border") == 0) {
                vl_css_layout_border_t border = construct_border(node, &rule->value);
                node->border[0] = node->border[1] = node->border[2] = node->border[3] = border;
                goto next;
            }
            if (strcmp(rule->property, "border-color") == 0 && rule->value.as.list) {
                int len = VL_DA_LENGTH(rule->value.as.list);
                switch (len) {
                case 0: break;
                case 1: {
                    vl_css_color_rgba_t rgba = vl_css_value_to_rgba(rule->value.as.list[0]);
                    for (int k = 0; k < 4; k++) node->border[k].color = rgba;
                    break;
                }
                case 2: {
                    vl_css_color_rgba_t rgba0 = vl_css_value_to_rgba(rule->value.as.list[0]);
                    vl_css_color_rgba_t rgba1 = vl_css_value_to_rgba(rule->value.as.list[1]);
                    node->border[0].color = node->border[2].color = rgba0;
                    node->border[1].color = node->border[3].color = rgba1;
                    break;
                }
                case 3: {
                    vl_css_color_rgba_t rgbat = vl_css_value_to_rgba(rule->value.as.list[0]);
                    vl_css_color_rgba_t rgbax = vl_css_value_to_rgba(rule->value.as.list[1]);
                    vl_css_color_rgba_t rgbab = vl_css_value_to_rgba(rule->value.as.list[2]);
                    node->border[0].color = rgbat;
                    node->border[1].color = node->border[3].color = rgbax;
                    node->border[2].color = rgbab;
                    break;
                }
                default:
                case 4: {
                    vl_css_color_rgba_t rgbat = vl_css_value_to_rgba(rule->value.as.list[0]);
                    vl_css_color_rgba_t rgbar = vl_css_value_to_rgba(rule->value.as.list[1]);
                    vl_css_color_rgba_t rgbab = vl_css_value_to_rgba(rule->value.as.list[2]);
                    vl_css_color_rgba_t rgbal = vl_css_value_to_rgba(rule->value.as.list[3]);
                    node->border[0].color = rgbat;
                    node->border[1].color = rgbar;
                    node->border[2].color = rgbab;
                    node->border[3].color = rgbal;
                    break;
                }
                }
                goto next;
            }
            if (strcmp(rule->property, "border-style") == 0) {
                if (rule->value.as.list) {
                    switch (VL_DA_LENGTH(rule->value.as.list)) {
                    case 0: break;
                    case 1: {
                        vl_css_layout_border_type_t type = get_border_type(rule->value.as.list);
                        node->border[0].type = node->border[1].type = node->border[2].type = node->border[3].type = type;
                        break;
                    }
                    case 2: {
                        vl_css_layout_border_type_t y = get_border_type(rule->value.as.list);
                        vl_css_layout_border_type_t x = get_border_type(rule->value.as.list + 1);
                        node->border[0].type = node->border[2].type = y;
                        node->border[1].type = node->border[3].type = x;
                        break;
                    }
                    case 3: {
                        vl_css_layout_border_type_t t = get_border_type(rule->value.as.list);
                        vl_css_layout_border_type_t x = get_border_type(rule->value.as.list + 1);
                        vl_css_layout_border_type_t b = get_border_type(rule->value.as.list + 2);
                        node->border[0].type = t;
                        node->border[1].type = node->border[3].type = x;
                        node->border[2].type = b;
                        break;
                    }
                    default:
                    case 4: {
                        vl_css_layout_border_type_t t = get_border_type(rule->value.as.list);
                        vl_css_layout_border_type_t r = get_border_type(rule->value.as.list + 1);
                        vl_css_layout_border_type_t b = get_border_type(rule->value.as.list + 2);
                        vl_css_layout_border_type_t l = get_border_type(rule->value.as.list + 3);
                        node->border[0].type = t;
                        node->border[1].type = r;
                        node->border[2].type = b;
                        node->border[3].type = l;
                        break;
                    }
                    }
                }
                goto next;
            }
            if (strcmp(rule->property, "border-width") == 0) {
                if (rule->value.as.list) {
                    switch (VL_DA_LENGTH(rule->value.as.list)) {
                    case 0: break;
                    case 1: {
                        float m = get_border_width(node, rule->value.as.list);
                        node->border[0].width = node->border[1].width =
                        node->border[2].width = node->border[3].width = m;
                        break;
                    }
                    case 2: {
                        float y = get_border_width(node, rule->value.as.list);
                        float x = get_border_width(node, rule->value.as.list + 1);
                        node->border[0].width = node->border[2].width = y;
                        node->border[1].width = node->border[3].width = x;
                        break;
                    }
                    case 3: {
                        float t = get_border_width(node, rule->value.as.list);
                        float x = get_border_width(node, rule->value.as.list + 1);
                        float b = get_border_width(node, rule->value.as.list + 2);
                        node->border[0].width = t;
                        node->border[1].width = node->border[3].width = x;
                        node->border[2].width = b;
                        break;
                    }
                    default:
                    case 4: {
                        float t = get_border_width(node, rule->value.as.list);
                        float r = get_border_width(node, rule->value.as.list + 1);
                        float b = get_border_width(node, rule->value.as.list + 2);
                        float l = get_border_width(node, rule->value.as.list + 3);
                        node->border[0].width = t;
                        node->border[1].width = r;
                        node->border[2].width = b;
                        node->border[3].width = l;
                        break;
                    }
                    }
                }
                goto next;
            }
            if (strcmp(rule->property, vl_sprintf_tmp("border-%s", sides[j])) == 0) {
                node->border[j] = construct_border(node, &rule->value);
                goto next;
            }
            if (strcmp(rule->property, vl_sprintf_tmp("border-%s-width", sides[j])) == 0 && VL_CSS_VALUE_IS_METRIC(rule->value)) {
                vl_css_size_metric_t processed_metric = vl_css_layout_node_process_metric(node, NULL, rule->value.as.metric1, 0);
                node->border[j].width = processed_metric.value;
                goto next;
            }
            if (strcmp(rule->property, vl_sprintf_tmp("border-%s-color", sides[j])) == 0 && VL_CSS_VALUE_COLOR_COMPATIBLE(rule->value)) {
                node->border[j].color = vl_css_value_to_rgba(rule->value);
                goto next;
            }
            if (strcmp(rule->property, vl_sprintf_tmp("border-%s-type", sides[j])) == 0) {
                node->border[j].type = get_border_type(&rule->value);
                goto next;
            }
        }

        next:
        continue;
    }
    for (int i = 0; i < VL_ARR_LEN(node->border); i++) {
        if (!node->border[i].type) node->border[i].width = 0;
    }
}

#endif // VELVET_CSS_BITS_CONSTRUCT_BORDERS_C