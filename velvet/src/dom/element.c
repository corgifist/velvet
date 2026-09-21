#include "velvet/dom/element.h"
#include "css/layout.h"
#include "css/style.h"
#include "support/color.h"
#include "graphics/render.h"
#include "support/math.h"
#include "support/memory.h"
#include "support/result.h"
#include "velvet/support/feature.h"
#include <string.h>
#include "web/web.h"

#include "velvet/dom/html/html.h"
#include "velvet/dom/head/head.h"
#include "velvet/dom/style/style.h"
#include "velvet/dom/div/div.h"
#include "velvet/dom/text/text.h"

typedef struct {
    const char *tag;
    vl_dom_element_new_func new_; // just to make sure we don't collide with the C++ new
} vl_dom_element_pair_t;

static const vl_dom_element_pair_t s_elements[] = {
    {"text", vl_dom_element_text_new},
    {"body", vl_dom_element_div_new},
    {"html", vl_dom_element_html_new},
    {"style", vl_dom_element_style_new},
    {"p", vl_dom_element_div_new},
    {"div", vl_dom_element_div_new},
    {"span", vl_dom_element_div_new},
    {"head", vl_dom_element_head_new},
    {"center", vl_dom_element_div_new},
    {"h1", vl_dom_element_div_new},
    {"h2", vl_dom_element_div_new},
    {"h3", vl_dom_element_div_new},
    {"h4", vl_dom_element_div_new},
    {"h5", vl_dom_element_div_new},
    {"h6", vl_dom_element_div_new},
    {"code", vl_dom_element_div_new}
};

static vl_vec2_t dom_to_css_size(vl_css_layout_node_t *node) {
    return vl_dom_element_get_content_size(node->owner);
}

vl_dom_element_t *vl_dom_element_new_(const char *tag, vl_source_location_t loc) {
    for (int i = 0; i < VL_ARR_LEN(s_elements); i++) {
        if (strcmp(s_elements[i].tag, tag) == 0) {
            vl_dom_element_t *element = s_elements[i].new_(tag, loc);
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

static void render_element_background(vl_dom_element_t *element) {
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
            vl_graphics_render_batch_rect_colored(web->render, VL_RECT(
                VL_VEC2(element->layout.bounds_offset.x, element->layout.bounds_offset.y), 
                VL_POINT_ADD(element->layout.size, VL_VEC2(element->layout.bounds_offset.z, element->layout.bounds_offset.w))), 
            NULL, VL_QUAD_COLOR(raw_color));
    }
}

static void render_element_border_solid(vl_dom_element_t *element, int side) {
    vl_web_t *web = element->owner->owner;
    vl_css_layout_border_t *top = element->layout.border;
    vl_css_layout_border_t *right = top + 1;
    vl_css_layout_border_t *bottom = right + 1;
    vl_css_layout_border_t *left = bottom + 1;
    vl_css_layout_border_t *border = top + side;
    float t = top->type ? top->width : 0;
    float r = right->type ? right->width : 0;
    float b = bottom->type ? bottom->width : 0;
    float l = left->type ? left->width : 0;
    vl_vec2_t size = element->layout.size;

    switch (side) {
    case 0: {
        vl_quad_t top_quad = VL_QUAD(
            VL_VEC2(0, 0), VL_VEC2(size.x, 0),
            VL_VEC2(l, t), VL_VEC2(size.x - r, t)
        );
        vl_graphics_render_batch_quad_colored(web->render, top_quad, NULL, VL_QUAD_COLOR(top->color));
        break;
    }
    case 1: {
        vl_quad_t right_quad = VL_QUAD(
            VL_VEC2(size.x - r, t), VL_VEC2(size.x, 0),
            VL_VEC2(size.x - r, size.y - b), VL_VEC2(size.x, size.y)
        );
        vl_graphics_render_batch_quad_colored(web->render, right_quad, NULL, VL_QUAD_COLOR(right->color));
        break;
    }
    case 2: {
        vl_quad_t bottom_quad = VL_QUAD(
            VL_VEC2(l, size.y - b), VL_VEC2(size.x - r, size.y - b),
            VL_VEC2(0, size.y), size
        );
        vl_graphics_render_batch_quad_colored(web->render, bottom_quad, NULL, VL_QUAD_COLOR(bottom->color));
        break;
    }
    case 3: {
        vl_quad_t left_quad = VL_QUAD(
            VL_VEC2(0, 0), VL_VEC2(l, t),
            VL_VEC2(0, size.y), VL_VEC2(l, size.y - b)
        );
        vl_graphics_render_batch_quad_colored(web->render, left_quad, NULL, VL_QUAD_COLOR(left->color));
        break;
    }
    }
}

static void render_element_border_double(vl_dom_element_t *element, int side) {
    vl_web_t *web = element->owner->owner;
    vl_css_layout_border_t *top = element->layout.border;
    vl_css_layout_border_t *right = top + 1;
    vl_css_layout_border_t *bottom = right + 1;
    vl_css_layout_border_t *left = bottom + 1;
    vl_css_layout_border_t *border = top + side;
    float t = top->type ? top->width : 0;
    float r = right->type ? right->width : 0;
    float b = bottom->type ? bottom->width : 0;
    float l = left->type ? left->width : 0;
    vl_vec2_t size = element->layout.size;

    switch (side) {
    case 0: {
        vl_quad_t top_quad1 = VL_QUAD(
            VL_VEC2(0, 0), VL_VEC2(size.x, 0),
            VL_VEC2(l / 3, t / 3), VL_VEC2(size.x - r / 3, t / 3)
        );
        vl_quad_t top_quad2 = VL_QUAD(
            VL_VEC2(l * 2 / 3, t * 2 / 3), VL_VEC2(size.x - r * 2 / 3, t * 2 / 3),
            VL_VEC2(l, t), VL_VEC2(size.x - r, t)
        );
        vl_graphics_render_batch_quad_colored(web->render, top_quad1, NULL, VL_QUAD_COLOR(top->color));
        vl_graphics_render_batch_quad_colored(web->render, top_quad2, NULL, VL_QUAD_COLOR(top->color));
        break;
    }
    case 1: {
        vl_quad_t right_quad1 = VL_QUAD(
            VL_VEC2(size.x - r / 3, t / 3), VL_VEC2(size.x, 0),
            VL_VEC2(size.x - r / 3, size.y - b / 3), VL_VEC2(size.x, size.y)
        );
        vl_quad_t right_quad2 = VL_QUAD(
            VL_VEC2(size.x - r, t), VL_VEC2(size.x - r * 2 / 3, t * 2 / 3),
            VL_VEC2(size.x - r, size.y - b), VL_VEC2(size.x - r * 2 / 3, size.y - b * 2 / 3)
        );
        vl_graphics_render_batch_quad_colored(web->render, right_quad1, NULL, VL_QUAD_COLOR(right->color));
        vl_graphics_render_batch_quad_colored(web->render, right_quad2, NULL, VL_QUAD_COLOR(right->color));
        break;
    }
    case 2: {
        vl_quad_t bottom_quad1 = VL_QUAD(
            VL_VEC2(l / 3, size.y - b / 3), VL_VEC2(size.x - r / 3, size.y - b / 3),
            VL_VEC2(0, size.y), size
        );
        vl_quad_t bottom_quad2 = VL_QUAD(
            VL_VEC2(l, size.y - b), VL_VEC2(size.x - r, size.y - b),
            VL_VEC2(l * 2 / 3, size.y - b * 2 / 3), VL_VEC2(size.x - r * 2 / 3, size.y - b * 2 / 3)
        );
        vl_graphics_render_batch_quad_colored(web->render, bottom_quad1, NULL, VL_QUAD_COLOR(bottom->color));
        vl_graphics_render_batch_quad_colored(web->render, bottom_quad2, NULL, VL_QUAD_COLOR(bottom->color));
        break;
    }
    case 3: {
        vl_quad_t left_quad1 = VL_QUAD(
            VL_VEC2(0, 0), VL_VEC2(l / 3, t / 3),
            VL_VEC2(0, size.y), VL_VEC2(l / 3, size.y - b / 3)
        );
        vl_quad_t left_quad2 = VL_QUAD(
            VL_VEC2(l * 2 / 3, t * 2 / 3), VL_VEC2(l, t),
            VL_VEC2(l * 2 / 3, size.y - b * 2 / 3), VL_VEC2(l, size.y - b)
        );
        vl_graphics_render_batch_quad_colored(web->render, left_quad1, NULL, VL_QUAD_COLOR(left->color));
        vl_graphics_render_batch_quad_colored(web->render, left_quad2, NULL, VL_QUAD_COLOR(left->color));
        break;
    }
    }
}

static void render_element_border(vl_dom_element_t *element, int side) {
    switch (element->layout.border[side].type) {
    case VL_CSS_LAYOUT_BORDER_SOLID: render_element_border_solid(element, side); break;
    case VL_CSS_LAYOUT_BORDER_DOUBLE: render_element_border_double(element, side); break;
    default: break;
    }
}

static void render_element_borders(vl_dom_element_t *element) {
    render_element_border(element, 0);
    render_element_border(element, 1);
    render_element_border(element, 2);
    render_element_border(element, 3);
}

static void render_element_highlight(vl_dom_element_t *element) {
    vl_web_t *web = element->owner->owner;
    vl_css_value_t velvet_element_highlight = vl_css_layout_node_get_property(&element->layout, 
        "--velvet-element-highlight", VL_CSS_VALUE_NONE()
    );
    if (velvet_element_highlight.type != VL_CSS_VALUE_NONE && VL_CSS_VALUE_COLOR_COMPATIBLE(velvet_element_highlight)) {
        vl_color_t highlight_color = vl_css_value_to_rgba(velvet_element_highlight);
        if (highlight_color.a != 0) {
            vl_graphics_render_batch_rect_colored(web->render, 
                VL_RECT(VL_VEC2(0, 0), element->layout.size), NULL, 
                VL_QUAD_COLOR(highlight_color)
            );
        }
    }
}

static void render_margin_highlight(vl_dom_element_t *element) {
    vl_web_t *web = element->owner->owner;
    vl_css_value_t velvet_margin_highlight = vl_css_layout_node_get_property(&element->layout, "--velvet-margin-highlight", VL_CSS_VALUE_NONE());
    if (velvet_margin_highlight.type != VL_CSS_VALUE_NONE && VL_CSS_VALUE_COLOR_COMPATIBLE(velvet_margin_highlight)) {
        vl_color_t margin_highlight_color = vl_css_value_to_rgba(velvet_margin_highlight);
        // margin-left
        vl_graphics_render_batch_rect_colored(web->render, 
            VL_RECT(VL_VEC2(-element->layout.margin.w, 0), VL_VEC2(0, element->layout.size.y)), NULL, 
            VL_QUAD_COLOR(margin_highlight_color)
        );
        // margin-right
        vl_graphics_render_batch_rect_colored(web->render, 
            VL_RECT(VL_VEC2(element->layout.size.x, 0), VL_VEC2(element->layout.size.x + element->layout.margin.y, element->layout.size.y)), NULL, 
            VL_QUAD_COLOR(margin_highlight_color)
        );
        // margin-top
        vl_graphics_render_batch_rect_colored(web->render, 
            VL_RECT(
                VL_VEC2(-element->layout.margin.w, -element->layout.margin.x),
                VL_VEC2(element->layout.size.x + element->layout.margin.y, 0)
            ), NULL, 
            VL_QUAD_COLOR(margin_highlight_color)
        );
        // margin-bottom
        vl_graphics_render_batch_rect_colored(web->render, 
            VL_RECT(
                VL_VEC2(-element->layout.margin.w, element->layout.size.y + element->layout.margin.x),
                VL_VEC2(element->layout.size.x + element->layout.margin.y, element->layout.size.y)
            ), NULL, 
            VL_QUAD_COLOR(margin_highlight_color)
        );
    }
}

vl_result_t vl_dom_element_render(vl_dom_element_t *element) {
    if (!element) return VL_ERROR;
    vl_dom_element_funcs_t *funcs = VL_DOM_ELEMENT_FUNCS(element);
    if (!funcs->render) return VL_SUCCESS;
    vl_web_t *web = element->owner->owner;
    render_element_background(element);
    render_element_borders(element);
    if (element->layout.pseudo_before) {
        vl_dom_element_render(element->layout.pseudo_before->owner);
    }
    vl_result_t result = funcs->render(element);
    render_element_highlight(element);
    render_margin_highlight(element);
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

static vl_result_t element_set_style(vl_dom_element_t *element, const char *css) {
    vl_css_inline_style_deinit(&element->layout.inline_style);
    vl_css_inline_style_init(&element->layout.inline_style, css);
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
    if (strcmp(property, "style") == 0) {
        if (type != VL_DOM_ELEMENT_PROPERTY_STRING) {
            return VL_ERROR;
        }
        return element_set_style(element, value);
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