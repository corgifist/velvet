#include "velvet/dom/behavior/text.h"
#include "css/style.h"
#include "font/shaper.h"
#include "graphics/color.h"
#include "graphics/geometry.h"
#include "support/da.h"
#include "support/result.h"
#include "velvet/web/web.h"
#include "web/fonts.h"
#include "support/math.h"

vl_result_t vl_dom_behavior_text_layout_new(vl_dom_element_t *element, 
    vl_dom_behavior_text_layout_t *layout, const char *text) {
    if (!element || !layout || !text) return VL_ERROR;
    if (!element->owner || !element->owner->owner) return VL_ERROR;
    vl_web_t *web = element->owner->owner;
    vl_web_fonts_t *fonts = &web->fonts;
    if (layout->glyphs) {
        VL_DA_FREE(layout->glyphs);
    }
    layout->glyphs = VL_DA_INIT(vl_dom_behavior_text_glyph_t);
    vl_font_shaper_run_t *run = vl_font_shaper_run_new(fonts->shaper);
    VL_DA(vl_font_shaper_font_ref_t*) font_stack = VL_DA_INIT(vl_font_shaper_font_ref_t*);
    VL_DA(vl_font_shaper_font_ref_t*) original_font_stack = fonts->shaper->font_stack;
    vl_web_sized_font_t *priority_font = vl_web_fonts_get_font(&web->fonts, layout->priority_font_family, 
        layout->font_blueprint.weight, layout->font_blueprint.height);
    vl_web_sized_font_t *default_arabic_font = vl_web_fonts_get_font(&web->fonts, "Noto Sans Arabic", 
        VL_WEB_FONT_REGULAR, layout->font_blueprint.height);
    if (default_arabic_font) {
        VL_DA_APPEND(font_stack, default_arabic_font->shaper_ref);
    }
    if (priority_font) {
        VL_DA_APPEND(font_stack, priority_font->shaper_ref);
    }
    fonts->shaper->font_stack = font_stack;
    vl_font_shaper_process(fonts->shaper, text, strlen(text));
    while (vl_font_shaper_shape(fonts->shaper, run)) {
        if (run->newline) {
            vl_dom_behavior_text_glyph_t newline = {0};
            newline.newline = true;
            VL_DA_APPEND(layout->glyphs, newline);
        }
        vl_font_shaper_glyph_t shaper_glyph = {0};
        while (vl_font_shaper_iterate(run, &shaper_glyph)) {
            vl_dom_behavior_text_glyph_t text_glyph = {0};
            text_glyph.id = shaper_glyph.id;
            text_glyph.advance_x = shaper_glyph.advance_x;
            text_glyph.advance_y = shaper_glyph.advance_y;
            text_glyph.x = shaper_glyph.x;
            text_glyph.y = shaper_glyph.y;
            text_glyph.font = run->font;
            VL_DA_APPEND(layout->glyphs, text_glyph);
        }
    }
    vl_font_shaper_run_free(run);
    VL_DA_FREE(font_stack);
    fonts->shaper->font_stack = original_font_stack;
    return VL_SUCCESS;
}

vl_result_t vl_dom_behavior_text_layout_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts, vl_dom_behavior_text_layout_t *layout) {
    if (!element->owner || !layout || !layout->glyphs) {
        return VL_ERROR;
    }
    vl_web_t *web = element->owner->owner;
    float base_x = 0;
    float base_y = 0;
    vl_css_value_t text_color = vl_css_style_get_property(&element->style, "color", VL_CSS_VALUE_RGBA(255, 255, 255, 1));
    vl_color_t normalized_color = VL_COLOR(
        text_color.as.rgba.r / 255.0f,
        text_color.as.rgba.g / 255.0f,
        text_color.as.rgba.b / 255.0f,
        text_color.as.rgba.a
    );
    vl_quad_colors_t quad_color = VL_QUAD_COLOR(normalized_color);
    for (int i = 0; i < VL_DA_LENGTH(layout->glyphs); i++) {
        vl_dom_behavior_text_glyph_t *glyph = layout->glyphs + i;
        vl_web_font_atlas_codepoint_t codepoint = {0};
        vl_web_fonts_find_glyph_id_with_font(&web->fonts, &codepoint, glyph->font, glyph->id);
        float x = base_x + glyph->x + codepoint.codepoint->x1;
        float y = base_y - glyph->y + codepoint.codepoint->y1;
        vl_graphics_render_batch_rect_colored_uv(web->render, VL_RECT_EX(
            x, y,
            x + codepoint.codepoint->w, y + codepoint.codepoint->h
        ), codepoint.atlas->brush, quad_color, codepoint.codepoint->uv);
        base_x += glyph->advance_x;
        base_y += glyph->advance_y;
    }
    return VL_SUCCESS;
}

vl_vec2_t vl_dom_behavior_text_layout_get_size(vl_dom_element_t *element, vl_dom_behavior_text_layout_t *layout) {
    if (!element || !layout) return VL_VEC2(0, 0);
    vl_vec2_t size = {0};
    vl_web_t *web = element->owner->owner;
    if (layout->glyphs) {
        float base_x = 0;
        float base_y = 0;
        float max_x = 0;
        float max_y = 0;
        for (int i = 0; i < VL_DA_LENGTH(layout->glyphs); i++) {
            vl_dom_behavior_text_glyph_t *glyph = layout->glyphs + i;
            vl_web_font_atlas_codepoint_t codepoint = {0};
            vl_web_fonts_find_glyph_id_with_font(&web->fonts, &codepoint, glyph->font, glyph->id);
            float x = base_x + glyph->x + codepoint.codepoint->x1;
            float y = base_y - glyph->y + codepoint.codepoint->y1;
            float x2 = x + codepoint.codepoint->w;
            float y2 = y + codepoint.codepoint->h;
            max_x = VL_MAX(max_x, x2);
            max_y = VL_MAX(max_y, y2);
            base_x += glyph->advance_x;
            base_y += glyph->advance_y;
        }
        size = (vl_vec2_t) {max_x, max_y};
    }
    return size;
}

vl_result_t vl_dom_behavior_text_layout_free(vl_dom_element_t *element, vl_dom_behavior_text_layout_t *layout) {
    if (!element || !layout) return VL_ERROR;
    VL_DA_FREE(layout->glyphs);
    return VL_SUCCESS;
}