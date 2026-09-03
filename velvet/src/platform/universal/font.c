#include "font/atlas.h"
#include "support/math.h"
#include "support/da.h"
#include "support/math.h"
#include <stddef.h>
#include "font_classifier.c"
#define STB_TRUETYPE_IMPLEMENTATION
#include "velvet/platform/universal/font.h"
#include "font/font.h"
#include "support/result.h"

vl_font_t *vl_font_universal_new(vl_platform_context_t *context, const char *name, int height, float density, const vl_byte_t *data, size_t data_length, vl_source_location_t loc) {
    if (!context) return NULL;
    vl_font_universal_t *font = VL_NEW(vl_font_universal_t, loc);
    if (!font) return NULL;
    font->base.name = name;
    font->base.context = context;
    font->base.height = height;
    font->base.density = density;

    font->data = data;
    font->data_length = data_length;

    printf("font data: %p\n", data);
    if (!stbtt_InitFont(&font->font, font->data, stbtt_GetFontOffsetForIndex(data, 0))) {
        goto err;
    }
    font->slim_scale = stbtt_ScaleForMappingEmToPixels(&font->font, height);
    font->scale = font->slim_scale * density;
    stbtt_GetFontVMetrics(&font->font, &font->base.ascent, &font->base.descent, &font->base.line_gap);
    font->base.ascent *= font->slim_scale;
    font->base.descent *= font->slim_scale;
    font->base.line_gap *= font->slim_scale;
    font->base.newline_advance = font->base.ascent - font->base.descent + font->base.line_gap;
    font->base.kind = classify_font(data, stbtt_GetFontOffsetForIndex(data, 0), &font->font);
    return (vl_font_t*) font;
    err:
    vl_free(font);
    return NULL;
}

vl_font_atlas_codepoint_t *vl_font_universal_rasterize_glyph_id(vl_font_t *font, vl_font_atlas_t *atlas, uint32_t glyph_id) {
    if (!font || !atlas) return NULL;
    if (atlas->format != VL_FONT_ATLAS_FORMAT_RRRR8) return NULL;
    if (atlas->full) return NULL;
    vl_font_universal_t *f = (vl_font_universal_t*) font;
    int advance_x, left_bearing;
    stbtt_GetGlyphHMetrics(&f->font, glyph_id, &advance_x, &left_bearing);
    int x1, y1, x2, y2;
    stbtt_GetGlyphBitmapBox(&f->font, glyph_id, f->scale, f->scale, &x1, &y1, &x2, &y2);
    float w = x2 - x1;
    float h = y2 - y1;
    if (atlas->cursor_y >= atlas->height) {
        return NULL;
    }
    if (atlas->cursor_x + w + 2 >= atlas->width) {
        atlas->cursor_x = 2;
        atlas->cursor_y += atlas->largest_glyph_on_line + 2;
        atlas->largest_glyph_on_line = 0;
    }
    if (atlas->cursor_x < 2) {
        atlas->cursor_x = 2; // we don't want to rasterize fonts near the atlas edge
    }
    if (atlas->cursor_y + h + 2 >= atlas->height) {
        atlas->full = true;
        return NULL;
    }
    vl_byte_t *pixels = (atlas->data + atlas->width * atlas->cursor_y) + atlas->cursor_x;
    stbtt_MakeGlyphBitmap(&f->font, pixels, w, h, atlas->width, f->scale, f->scale, glyph_id);
    float bx1 = atlas->cursor_x;
    float by1 = atlas->cursor_y;
    float bx2 = bx1 + w;
    float by2 = by1 + h;
    vl_font_atlas_codepoint_t result = {0};
    result.owner = font;
    result.w = w / font->density;
    result.h = h / font->density;
    result.glyph_id = glyph_id;
    
    float lb = left_bearing * f->slim_scale;
    float ax = advance_x * f->slim_scale;
    result.advance_x = ax;
    result.x1 = lb;
    result.y1 = font->ascent + y1 / font->density;
    result.x2 = lb + w / font->density;
    result.y2 = font->ascent + ((float) (y1 + h)) / font->density;

    float aw = atlas->width;
    float ah = atlas->height;
    result.uv.tl = VL_POINT(bx1 / aw, by1 / ah);
    result.uv.tr = VL_POINT(bx2 / aw, by1 / ah);
    result.uv.br = VL_POINT(bx2 / aw, by2 / ah);
    result.uv.bl = VL_POINT(bx1 / aw, by2 / ah);
    atlas->cursor_x += w + 2;
    atlas->largest_glyph_on_line = VL_MAX(h, atlas->largest_glyph_on_line);
    if (atlas->cursor_x + font->height * font->density >= atlas->width
            && atlas->cursor_y + font->height * font->density >= atlas->height) {
        atlas->full = true;
    }
    return VL_DA_APPEND(atlas->codepoints, result);
}

uint32_t vl_font_universal_get_glyph_id_by_codepoint(vl_font_t *font, uint32_t codepoint) {
    if (!font) return 0;
    vl_font_universal_t *f = (vl_font_universal_t*) font;
    return stbtt_FindGlyphIndex(&f->font, codepoint);
}

float vl_font_universal_get_kern_advance(vl_font_t *font, uint32_t codepoint_a, uint32_t codepoint_b) {
    if (codepoint_a == 0 || codepoint_b == 0) return 0;
    vl_font_universal_t *f = (vl_font_universal_t*) font;
    return ((float) stbtt_GetGlyphKernAdvance(&f->font, codepoint_a, codepoint_b)) * f->slim_scale;
}

vl_vec2_t vl_font_universal_get_text_size_ex(vl_font_t *font, const char *text, size_t text_length) {
    vl_font_universal_t *f = (vl_font_universal_t*) font;
    float x = 0;
    float y = 0;
    float base_x = 0;
    float base_y = 0;
    for (size_t i = 0; i < text_length; i++) {
        int c = text[i];
        if (c == '\n') {
            base_y += font->newline_advance;
            base_x = 0;
            y = VL_MAX(y, base_y);
            continue;
        }
        int ax, lsb;
        stbtt_GetCodepointHMetrics(&f->font, c, &ax, &lsb);
        int x1, y1, x2, y2;
        stbtt_GetCodepointBitmapBox(&f->font, c, f->slim_scale, f->slim_scale, &x1, &y1, &x2, &y2);
        int w = x2 - x1;
        int h = y2 - y1;
        base_x += ax * f->slim_scale;
        float bx = base_x + (lsb + w) * f->slim_scale;
        float by = base_y + f->base.ascent - f->base.descent + f->base.line_gap + (y1 + h) / font->density;
        x = VL_MAX(bx, x);
        y = VL_MAX(by, y);
        if (i != text_length - 1)
            base_x += stbtt_GetGlyphKernAdvance(&f->font, c, text[i + 1]) * f->slim_scale;
    }
    return VL_VEC2(x, y);
}

vl_result_t vl_font_universal_free(vl_font_t *font) {
    if (!font) return VL_ERROR;
    // nothing to free for us!
    return VL_SUCCESS;
}