#include "font/atlas.h"
#include "support/da.h"
#include "support/math.h"
#include <stddef.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "velvet/platform/universal/font.h"
#include "font/font.h"
#include "support/result.h"
#include <math.h>

#define ifloor(X) ((int) floor(X))

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

    if (!stbtt_InitFont(&font->font, font->data, 0)) {
        goto err;
    }
    // font->base.height = height = height * density;
    // font->base.density = density = 1;
    font->slim_scale = stbtt_ScaleForPixelHeight(&font->font, height);
    font->scale = font->slim_scale * density;
    stbtt_GetFontVMetrics(&font->font, &font->ascent, &font->descent, &font->line_gap);

    return (vl_font_t*) font;
    err:
    vl_free(font);
    return NULL;
}

vl_font_atlas_codepoint_t *vl_font_universal_rasterize_codepoint(vl_font_t *font, vl_font_atlas_t *atlas, uint32_t codepoint) {
    if (!font || !atlas) return NULL;
    if (atlas->format != VL_FONT_ATLAS_FORMAT_RRRR8) return NULL;
    vl_font_universal_t *f = (vl_font_universal_t*) font;
    int advance_x, left_bearing;
    stbtt_GetCodepointHMetrics(&f->font, codepoint, &advance_x, &left_bearing);
    int x1, y1, x2, y2;
    stbtt_GetCodepointBitmapBox(&f->font, codepoint, f->scale, f->scale, &x1, &y1, &x2, &y2);
    float w = x2 - x1;
    float h = y2 - y1;
    if (atlas->cursor_y >= atlas->height) {
        return NULL;
    }
    if (atlas->cursor_x + w >= atlas->width) {
        atlas->cursor_x = 0;
        atlas->cursor_y += font->height * font->density;
    }
    if (atlas->cursor_y + h >= atlas->height) {
        // atlas is full
        atlas->cursor_y += h;
        return NULL;
    }
    vl_byte_t *pixels = (atlas->data + atlas->width * atlas->cursor_y) + atlas->cursor_x;
    stbtt_MakeCodepointBitmap(&f->font, pixels, w, h, atlas->width, f->scale, f->scale, codepoint);
    float bx1 = atlas->cursor_x;
    float by1 = atlas->cursor_y;
    float bx2 = bx1 + w;
    float by2 = by1 + h;
    vl_font_atlas_codepoint_t result = {0};
    result.owner = font;
    result.w = w / font->density;
    result.h = h / font->density;
    result.codepoint = codepoint;
    
    float as = f->ascent * f->slim_scale;
    float lb = left_bearing * f->slim_scale;
    float ax = advance_x * f->slim_scale;
    result.advance_x = ax;
    result.x1 = lb;
    result.y1 = as + y1 / font->density;
    result.x2 = lb + w / font->density;
    result.y2 = as + (y1 + h) / font->density;

    float aw = atlas->width;
    float ah = atlas->height;
    result.uv.tl = VL_POINT(bx1 / aw, by1 / ah);
    result.uv.tr = VL_POINT(bx2 / aw, by1 / ah);
    result.uv.br = VL_POINT(bx2 / aw, by2 / ah);
    result.uv.bl = VL_POINT(bx1 / aw, by2 / ah);
    atlas->cursor_x += w + 1;
    return VL_DA_APPEND(atlas->codepoints, result);
}

float vl_font_universal_get_kern_advance(vl_font_t *font, uint32_t codepoint_a, uint32_t codepoint_b) {
    if (!font) return 0;
    if (codepoint_a == 0 || codepoint_b == 0) return 0;
    vl_font_universal_t *f = (vl_font_universal_t*) font;
    return ((float) stbtt_GetGlyphKernAdvance(&f->font, codepoint_a, codepoint_b)) * f->slim_scale;
}

vl_result_t vl_font_universal_free(vl_font_t *font) {
    if (!font) return VL_ERROR;
    // nothing to free for us!
    return VL_SUCCESS;
}