#include "velvet/dom/element.h"
#include "css/style.h"
#include "css/stylesheet.h"
#include "os/window.h"
#include "support/memory.h"
#include "support/result.h"
#include "velvet/support/feature.h"
#include <string.h>
#include "web/web.h"

#if VL_FEATURE(DOM_TEXT_NODE)
    #include "velvet/dom/text/text.h"
#endif // VL_FEATURE(DOM_TEXT_NODE)

#if VL_FEATURE(DOM_BODY_NODE)
    #include "velvet/dom/body/body.h"
#endif // VL_FEATURE(DOM_BODY_NODE)

#if VL_FEATURE(DOM_HTML_NODE)
    #include "velvet/dom/html/html.h"
#endif // VL_FEATURE(DOM_HTML_NODE)

#include "velvet/dom/style/style.h"

typedef struct {
    const char *tag;
    vl_dom_element_new_func new_; // just to make sure we don't collide with the C++ new
} vl_dom_element_pair_t;

static const vl_dom_element_pair_t s_elements[] = {
#if VL_FEATURE(DOM_TEXT_NODE)
    {"text", vl_dom_element_text_new},
    {"body", vl_dom_element_body_new},
    {"html", vl_dom_element_html_new},
    {"style", vl_dom_element_style_new}
#endif // VL_FEATURE(DOM_TEXT_NODE)
};

vl_dom_element_t *vl_dom_element_new_(const char *tag, vl_source_location_t loc) {
    for (int i = 0; i < VL_ARR_LEN(s_elements); i++) {
        if (strcmp(s_elements[i].tag, tag) == 0) {
            vl_dom_element_t *element = s_elements[i].new_(loc);
            if (element) {
                element->element_selector.id_chain = VL_DA_INIT(vl_css_class_id_t);
                vl_css_class_id_t *id = VL_DA_PUSH(element->element_selector.id_chain, vl_css_class_id_t);
                id->type = VL_CSS_CLASS_ID_ELEMENT;
                id->name = VL_DA_INIT_FROM_STRING(tag);
            }
            return element;
        }
    }
    return NULL;
}

vl_result_t vl_dom_element_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts) {
    if (!element) return VL_ERROR;
    vl_dom_element_funcs_t *funcs = VL_DOM_ELEMENT_FUNCS(element);
    if (!funcs->render) return VL_SUCCESS;
    return funcs->render(element, opts);
}

vl_result_t vl_dom_element_set_string(vl_dom_element_t *element, const char *property, const char *value) {
    return vl_dom_element_set_property(element, property, VL_DOM_ELEMENT_PROPERTY_STRING, value);
}

vl_result_t vl_dom_element_update_style(vl_dom_element_t *element) {
    if (!element || !element->tag || !element->owner || !element->owner->owner) return VL_ERROR;
    vl_web_t *web = element->owner->owner;
    vl_css_style_deinit(&element->style);
    vl_css_style_init(&element->style);
    if (strcmp(element->tag, "text") == 0) {
        vl_dom_element_t *parent = element->parent;
        if (parent->style.applied_rules) {
            for (int i = 0; i < VL_DA_LENGTH(parent->style.applied_rules); i++) {
                *VL_DA_PUSH(element->style.applied_rules, vl_css_rule_t*) = parent->style.applied_rules[i];
            }
        }
        return VL_SUCCESS;
    }

    VL_DA(vl_css_class_t*) matched_classes = NULL;
    vl_css_stylesheet_broad_query(&web->stylesheet, &element->element_selector, &matched_classes);
    if (element->class_selectors) {
        for (int i = 0; i < VL_DA_LENGTH(element->class_selectors); i++) {
            vl_css_stylesheet_broad_query(&web->stylesheet, element->class_selectors + i, &matched_classes);
        }
    }
    if (matched_classes) {
        for (int i = 0; i < VL_DA_LENGTH(matched_classes); i++) {
            vl_css_style_t tmp_style = {0};
            vl_css_style_from_class(&tmp_style, matched_classes[i]);
            vl_css_style_merge(&element->style, &tmp_style);
            vl_css_style_deinit(&tmp_style);
        }
    }
    VL_DA_FREE(matched_classes);
    vl_css_style_print(&element->style);
    if (element->children) {
        for (int i = 0; i < VL_DA_LENGTH(element->children); i++) {
            vl_dom_element_update_style(element->children[i]);
        }
    }
    return VL_SUCCESS;
}

static vl_result_t element_set_class_name(vl_dom_element_t *element, const char *class_string) {
    if (element->class_selectors) {
        for (int i = 0; i < VL_DA_LENGTH(element->class_selectors); i++) {
            vl_css_class_selector_deinit(element->class_selectors + i);
        }
        VL_DA_FREE(element->class_selectors);
    }
    element->class_selectors = VL_DA_INIT(vl_css_class_selector_t);
    const char *begin = class_string;
    const char *end = class_string;
    VL_DA_FREE(element->class_name);
    element->class_name = VL_DA_INIT_FROM_STRING(class_string);
    const char *class_begin = element->class_name;
    const char *class_end = element->class_name;
    const char *limit = element->class_name + VL_DA_LENGTH(element->class_name);
    while (class_end < limit) {
        if (*class_end == ' ' || *class_end == '\0') {
            vl_css_class_selector_t selector = {0};
            selector.id_chain = VL_DA_INIT(vl_css_class_id_t);
            vl_css_class_id_t id = {0};
            id.type = VL_CSS_CLASS_ID_CLASS;
            id.name = VL_DA_INIT_FROM_STRING_WITH_SIZE(class_begin, class_end - class_begin);
            VL_DA_APPEND(selector.id_chain, id);
            VL_DA_APPEND(element->class_selectors, selector);
            class_end++;
            class_begin = class_end;
        }
        class_end++;
    }
    for (int i = 0; i < VL_DA_LENGTH(element->class_selectors); i++) {
        vl_css_class_selector_print(element->class_selectors + i);
    }
    return VL_SUCCESS;
}

vl_result_t vl_dom_element_set_property(vl_dom_element_t *element, const char *property, vl_dom_element_property_type_t type, const void *value) {
    if (!element || !property) return VL_ERROR;
    vl_dom_element_funcs_t *funcs = VL_DOM_ELEMENT_FUNCS(element);
    if (strcmp(property, "className") == 0) {
        if (type != VL_DOM_ELEMENT_PROPERTY_STRING) {
            return VL_ERROR;
        }
        return element_set_class_name(element, value);
    }
    if (!funcs->set_property) return VL_ERROR;
    return funcs->set_property(element, property, type, value);
}

static float process_metric(float parent_size, vl_css_size_metric_t metric) {
    if (metric.type == VL_CSS_SIZE_METRIC_PERCENTAGE) return parent_size * metric.value;
    return metric.value;
}

vl_vec2_t vl_dom_element_get_metric2(vl_dom_element_t *element, vl_dom_element_metric_type_t metric) {
    if (!element) return VL_VEC2(0, 0);
    vl_dom_element_funcs_t *funcs = VL_DOM_ELEMENT_FUNCS(element);
    vl_os_window_t *window = element->owner->owner->render->owner;
    if (element->calculating_metric) return VL_VEC2(0, 0);
    element->calculating_metric = true;
    vl_vec2_t result = {0};
    vl_vec2_t parent_size = {0};
    if (element->tag && strcmp(element->tag, "html") == 0 && element->parent == NULL) {
        parent_size = VL_VEC2(window->io.window_size.x, 0);
    } else {
        parent_size = vl_dom_element_get_metric2(element->parent, VL_DOM_ELEMENT_METRIC_SIZE);
    }
    switch (metric) {
    case VL_DOM_ELEMENT_METRIC_SIZE: {
        vl_css_value_t width_value = vl_css_style_get_property(&element->style, "width", 
            VL_CSS_VALUE_METRIC1(VL_CSS_SIZE_PERCENTAGE(100))
        );
        vl_vec2_t content_size = funcs->get_metric2 ? funcs->get_metric2(element, metric) : (vl_vec2_t) {0};
        result.x = process_metric(parent_size.x, width_value.as.metric1);
        result.y = content_size.y;
        break;
    }
    default: {
        if (funcs->get_metric2) result = funcs->get_metric2(element, metric);
        break;
    }
    }
    element->calculating_metric = false;
    return result;
}

vl_result_t vl_dom_element_free(vl_dom_element_t *element) {
    if (!element) return VL_ERROR;
    vl_css_class_selector_deinit(&element->element_selector);
    if (element->class_selectors) {
        for (int i = 0; i < VL_DA_LENGTH(element->class_selectors); i++) {
            vl_css_class_selector_deinit(element->class_selectors + i);
        }
        VL_DA_FREE(element->class_selectors);
    }
    if (element->class_name) {
        VL_DA_FREE(element->class_name);
    }
    if (element->children) {
        for (int i = 0; i < VL_DA_LENGTH(element->children); i++) {
            vl_dom_element_free(element->children[i]);
        }
        VL_DA_FREE(element->children);
    }
    vl_css_style_deinit(&element->style);
    return VL_DOM_ELEMENT_FUNCS(element)->free(element);
}