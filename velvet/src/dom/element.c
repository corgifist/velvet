#include "velvet/dom/element.h"
#include "css/layout.h"
#include "css/style.h"
#include "support/color.h"
#include "graphics/render.h"
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
#include "velvet/dom/p/p.h"
#include "velvet/dom/div/div.h"
#include "velvet/dom/span/span.h"

typedef struct {
    const char *tag;
    vl_dom_element_new_func new_; // just to make sure we don't collide with the C++ new
} vl_dom_element_pair_t;

static const vl_dom_element_pair_t s_elements[] = {
#if VL_FEATURE(DOM_TEXT_NODE)
    {"text", vl_dom_element_text_new},
    {"body", vl_dom_element_body_new},
    {"html", vl_dom_element_html_new},
    {"style", vl_dom_element_style_new},
    {"p", vl_dom_element_p_new},
    {"div", vl_dom_element_div_new},
    {"span", vl_dom_element_span_new}
#endif // VL_FEATURE(DOM_TEXT_NODE)
};

static vl_vec2_t dom_to_css_size(vl_css_layout_node_t *node) {
    return vl_dom_element_get_content_size(node->owner);
}

vl_dom_element_t *vl_dom_element_new_(const char *tag, vl_source_location_t loc) {
    for (int i = 0; i < VL_ARR_LEN(s_elements); i++) {
        if (strcmp(s_elements[i].tag, tag) == 0) {
            vl_dom_element_t *element = s_elements[i].new_(loc);
            if (element) {
                vl_css_layout_node_init(&element->layout, s_elements[i].tag);
                element->layout.owner = element;
                element->layout.get_content_size = dom_to_css_size;
            }
            return element;
        }
    }
    return NULL;
}

vl_result_t vl_dom_element_process(vl_dom_element_t *element) {
    if (!element) return VL_ERROR;
    return vl_css_layout_node_process(&element->layout);
}

vl_result_t vl_dom_element_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts) {
    if (!element) return VL_ERROR;
    vl_dom_element_funcs_t *funcs = VL_DOM_ELEMENT_FUNCS(element);
    if (!funcs->render) return VL_SUCCESS;
    vl_web_t *web = element->owner->owner;
    vl_css_value_t background_color = vl_css_layout_node_get_property(&element->layout, 
        "background-color", VL_CSS_VALUE_RGBA(0, 0, 0, 0)
    );
    bool suitable_as_bg_color = VL_CSS_VALUE_COLOR_COMPATIBLE(background_color);
    bool is_body = (element->tag && (strcmp(element->tag, "body") == 0));
    if (suitable_as_bg_color && is_body) {
        vl_color_t raw_color = vl_css_value_to_rgba(background_color);
        vl_css_layout_node_t *web_root = element->layout.parent->parent;
        if (raw_color.a != 0) 
            vl_graphics_render_batch_rect_colored(web->render, VL_RECT_EX(0, 0, web_root->size.x, web_root->size.y), NULL, VL_QUAD_COLOR(raw_color));
    }
    vl_graphics_render_push_translate(web->render, element->layout.position);
    if (suitable_as_bg_color && !is_body) {
        vl_color_t raw_color = vl_css_value_to_rgba(background_color);
        if (raw_color.a != 0)
            vl_graphics_render_batch_rect_colored(web->render, VL_RECT(VL_VEC2(0), VL_POINT_ADD(element->layout.size, element->layout.render_offset)), NULL, VL_QUAD_COLOR(raw_color));
    }
    vl_result_t result = funcs->render(element, opts);
    vl_css_value_t velvet_element_highlight = vl_css_layout_node_get_property(&element->layout, 
        "--velvet-element-highlight", VL_CSS_VALUE_RGBA(0, 0, 0, 0)
    );
    if (VL_CSS_VALUE_COLOR_COMPATIBLE(velvet_element_highlight)) {
        vl_color_t highlight_color = vl_css_value_to_rgba(velvet_element_highlight);
        if (highlight_color.a != 0) 
            vl_graphics_render_batch_rect_colored(web->render, 
                VL_RECT(VL_VEC2(0, 0), element->layout.size), NULL, 
                VL_QUAD_COLOR(highlight_color)
            );
    }
    vl_graphics_render_pop_transform(web->render);
    return result;
}

vl_result_t vl_dom_element_set_string(vl_dom_element_t *element, const char *property, const char *value) {
    return vl_dom_element_set_property(element, property, VL_DOM_ELEMENT_PROPERTY_STRING, value);
}

static vl_result_t element_set_class_name(vl_dom_element_t *element, const char *class_string) {
    if (element->layout.affecting_selectors) {
        for (int i = 0; i < VL_DA_LENGTH(element->layout.affecting_selectors); i++) {
            vl_css_class_selector_deinit(element->layout.affecting_selectors + i);
        }
        VL_DA_FREE(element->layout.affecting_selectors);
    }
    element->layout.affecting_selectors = VL_DA_INIT(vl_css_class_selector_t);
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
            VL_DA_APPEND(element->layout.affecting_selectors, selector);
            class_end++;
            class_begin = class_end;
        }
        class_end++;
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

vl_vec2_t vl_dom_element_get_content_size(vl_dom_element_t *element) {
    if (!element) return VL_VEC2(0, 0);
    vl_dom_element_funcs_t *funcs = VL_DOM_ELEMENT_FUNCS(element);
    return funcs->get_content_size ? funcs->get_content_size(element) : VL_VEC2(0, 0);
}

vl_result_t vl_dom_element_free(vl_dom_element_t *element) {
    if (!element) return VL_ERROR;
    vl_css_layout_node_deinit(&element->layout);
    if (element->class_name) {
        VL_DA_FREE(element->class_name);
    }
    if (element->children) {
        for (int i = 0; i < VL_DA_LENGTH(element->children); i++) {
            vl_dom_element_free(element->children[i]);
        }
        VL_DA_FREE(element->children);
    }
    return VL_DOM_ELEMENT_FUNCS(element)->free(element);
}