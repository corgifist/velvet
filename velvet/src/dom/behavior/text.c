#include "velvet/dom/behavior/text.h"
#include "font/shaper.h"
#include "support/da.h"
#include "support/result.h"
#include "velvet/web/web.h"
#include "web/fonts.h"


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

vl_result_t vl_dom_behavior_text_layout_free(vl_dom_element_t *element, vl_dom_behavior_text_layout_t *layout) {
    if (!element || !layout) return VL_ERROR;
    VL_DA_FREE(layout->glyphs);
    return VL_SUCCESS;
}