#include "velvet/font/font.h"
#include "font/atlas.h"
#include "support/math.h"
#include "platform/universal/font.h"
#include "support/result.h"
#include "vendor/utf8.h"

vl_font_t *vl_font_new_(vl_platform_context_t *context, const char *name, int height, float density, const vl_byte_t *data, size_t data_length, vl_source_location_t loc) {
    if (!context || !context->font_new) return NULL;
    return context->font_new(context, name, height, density, data, data_length, loc);
}

vl_font_atlas_codepoint_t *vl_font_rasterize_codepoint(vl_font_t *font, vl_font_atlas_t *atlas, uint32_t codepoint) {
    uint32_t glyph_id = vl_font_get_glyph_id_by_codepoint(font, codepoint);
    return vl_font_universal_rasterize_glyph_id(font, atlas, glyph_id);
}

vl_font_atlas_codepoint_t *vl_font_rasterize_glyph_id(vl_font_t *font, vl_font_atlas_t *atlas, uint32_t glyph_id) {
    if (!font || !atlas || !font->context || !font->context->font_rasterize_glyph_id) return NULL;
    return font->context->font_rasterize_glyph_id(font, atlas, glyph_id);
}

vl_result_t vl_font_rasterize_codepoint_range(vl_font_t *font, vl_font_atlas_t *atlas, uint32_t begin, uint32_t end) {
    if (!font || !atlas) return VL_ERROR;
    for (uint32_t c = begin; c <= end; c++) {
        if (vl_font_atlas_find_glyph_id(atlas, font, vl_font_get_glyph_id_by_codepoint(font, c))) {
            // already rasterized
            continue;
        }
        if (!vl_font_rasterize_codepoint(font, atlas, c)) return VL_ERROR;
    }
    return VL_SUCCESS;
}

vl_result_t vl_font_rasterize_codepoints(vl_font_t *font, vl_font_atlas_t *atlas, const char *codepoints) {
    if (!font || !atlas || !codepoints) return VL_ERROR;
    size_t len = strlen(codepoints);
    size_t i = 0;
    UChar32 codepoint = 0;
    while (true) {
        U8_NEXT(codepoints, i, len, codepoint);
        if (codepoint <= 0) break;
        if (vl_font_atlas_find_glyph_id(atlas, font, vl_font_get_glyph_id_by_codepoint(font, codepoint))) continue;
        if (!vl_font_rasterize_codepoint(font, atlas, codepoint)) return VL_ERROR;
    }
    return VL_SUCCESS;
}

uint32_t vl_font_get_glyph_id_by_codepoint(vl_font_t *font, uint32_t codepoint) {
    if (!font || !font->context || !font->context->font_get_glyph_id_by_codepoint) return 0;
    return font->context->font_get_glyph_id_by_codepoint(font, codepoint);
}

float vl_font_get_kern_advance(vl_font_t *font, uint32_t codepoint_a, uint32_t codepoint_b) {
    if (!font || !font->context || !font->context->font_get_kern_advance) return 0;
    return font->context->font_get_kern_advance(font, codepoint_a, codepoint_b);
}

vl_vec2_t vl_font_get_text_size(vl_font_t *font, const char *text) {
    return vl_font_get_text_size_ex(font, text, strlen(text));
}

vl_vec2_t vl_font_get_text_size_ex(vl_font_t *font, const char *text, size_t text_length) {
    if (!font || !text || !text_length || !font->context || !font->context->font_get_text_size_ex) return VL_VEC2(0, 0);
    return font->context->font_get_text_size_ex(font, text, text_length);
}

vl_result_t vl_font_free(vl_font_t *font) {
    if (!font) return VL_ERROR;
    if (font->context && font->context->font_free) {
        font->context->font_free(font);
    }
    vl_free(font);
    return VL_SUCCESS;
}