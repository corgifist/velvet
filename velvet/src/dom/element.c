#include "velvet/dom/element.h"
#include "css/style.h"
#include "css/stylesheet.h"
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
            return s_elements[i].new_(loc);
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
    vl_css_class_t *element_class = vl_css_stylesheet_find_class(&web->stylesheet, element->tag);
    if (element_class) {
        for (int i = 0; i < VL_DA_LENGTH(element_class->rules); i++) {
            *VL_DA_PUSH(element->style.applied_rules, vl_css_rule_t*) = element_class->rules + i;
        }
    }
    if (element->children) {
        for (int i = 0; i < VL_DA_LENGTH(element->children); i++) {
            vl_dom_element_update_style(element->children[i]);
        }
    }
    return VL_SUCCESS;
}

static vl_result_t element_set_class_name(vl_dom_element_t *element, const char *class_string) {
    const char *begin = class_string;
    const char *end = class_string;
    VL_DA_FREE(element->class_name);
    element->class_name = VL_DA_INIT_FROM_STRING(class_string);
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

vl_result_t vl_dom_element_free(vl_dom_element_t *element) {
    if (!element) return VL_ERROR;
    if (element->children) {
        for (int i = 0; i < VL_DA_LENGTH(element->children); i++) {
            vl_dom_element_free(element->children[i]);
        }
        VL_DA_FREE(element->children);
    }
    vl_css_style_deinit(&element->style);
    return VL_DOM_ELEMENT_FUNCS(element)->free(element);
}