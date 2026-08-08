#include "velvet/dom/text/text.h"
#include "dom/behavior/text.h"
#include "dom/dom.h"
#include "dom/element.h"
#include "font/font.h"
#include "font/shaper.h"
#include "graphics/brush.h"
#include "graphics/color.h"
#include "graphics/render.h"
#include "support/da.h"
#include "support/memory.h"
#include "support/result.h"
#include "support/managed_assert.h"
#include "web/fonts.h"
#include "web/web.h"

vl_dom_element_t *vl_dom_element_text_new(vl_source_location_t loc) {
    vl_dom_element_funcs_t *funcs = vl_malloc(sizeof(vl_dom_element_funcs_t) + sizeof(vl_dom_element_text_t));
    if (!funcs) return NULL;
    funcs->render = vl_dom_element_text_render;
    funcs->set_property = vl_dom_element_text_set_property;
    funcs->get_metric2 = NULL;
    funcs->free = vl_dom_element_text_free;
    vl_dom_element_text_t *element = VL_PTR_FORWARD(funcs, sizeof(*funcs));
    element->text = NULL;
    element->base.children = NULL;
    element->base.owner = NULL;
    element->base.tag = "text";
    element->layout.priority_font_family = "Roboto";
    element->layout.font_blueprint.height = 32;
    element->layout.font_blueprint.weight = VL_WEB_FONT_REGULAR;
    return (vl_dom_element_t*) element;
}

vl_result_t vl_dom_element_text_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    if (!text->text) return VL_ERROR;
    vl_dom_t *owner = element->owner;
    vl_web_t *web = owner->owner;

    if (!text->layout.glyphs || text->rebuild_layout) {
        vl_dom_behavior_text_layout_free(element, &text->layout);
        vl_dom_behavior_text_layout_new(element, &text->layout, text->text);
    }
    float base_x = 0;
    float base_y = 0;
    vl_graphics_brush_t *brush = NULL;
    for (int i = 0; i < VL_DA_LENGTH(text->layout.glyphs); i++) {
        vl_dom_behavior_text_glyph_t *glyph = text->layout.glyphs + i;
        vl_web_font_atlas_codepoint_t codepoint = {0};
        vl_web_fonts_find_glyph_id_with_font(&web->fonts, &codepoint, glyph->font, glyph->id);
        float x = base_x + glyph->x + codepoint.codepoint->x1;
        float y = base_y - glyph->y + codepoint.codepoint->y1;
        vl_graphics_render_batch_rect_colored_uv(web->render, VL_RECT_EX(
            x, y,
            x + codepoint.codepoint->w, y + codepoint.codepoint->h
        ), codepoint.atlas->brush, VL_QUAD_WHITE, codepoint.codepoint->uv);
        base_x += glyph->advance_x;
        base_y += glyph->advance_y;
        brush = codepoint.atlas->brush;
    }
    vl_graphics_render_batch_end(web->render);
    vl_graphics_render_batch_begin(web->render);
    vl_graphics_render_clear(web->render, VL_BLACK);
    vl_graphics_render_batch_rect(
        web->render, VL_RECT_EX(0, 0, 512, 512), brush
    );
    return VL_SUCCESS;
}

vl_result_t vl_dom_element_text_set_property(vl_dom_element_t *element, const char *property, vl_dom_element_property_type_t type, const void *value) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    if (strcmp(property, "innerText") == 0) {
        if (type != VL_DOM_ELEMENT_PROPERTY_STRING) VL_ASSERT(0 && "innerText property requires a STRING");
        if (text->text) {
            VL_DA_FREE(text->text);
        }
        size_t len = strlen(value);
        text->text = VL_DA_INIT_WITH_CAPACITY(char, len + 1);
        text->text[len] = '\0';
        memcpy(text->text, value, len);
        text->rebuild_layout = true;
        return VL_SUCCESS;
    }
    return VL_ERROR;
}

vl_result_t vl_dom_element_text_free(vl_dom_element_t *element) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    VL_DA_FREE(text->text);
    vl_free(element);
    return VL_SUCCESS;
}