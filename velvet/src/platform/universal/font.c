#include "font/atlas.h"
#include "support/da.h"
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
    font->scale = stbtt_ScaleForPixelHeight(&font->font, (float) height);
    font->dense_scale = stbtt_ScaleForPixelHeight(&font->font, ((float) height) * density);
    printf("%f %f\n", font->scale, font->dense_scale);
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
    int nx1, ny1, nx2, ny2;
    int dx1, dy1, dx2, dy2;
    stbtt_GetCodepointBitmapBox(&f->font, codepoint, f->scale, f->scale, &nx1, &ny1, &nx2, &ny2);
    stbtt_GetCodepointBitmapBox(&f->font, codepoint, f->dense_scale, f->dense_scale, &dx1, &dy1, &dx2, &dy2);
    int nw = nx2 - nx1;
    int nh = ny2 - ny1;
    int dw = dx2 - dx1;
    int dh = dy2 - dy1;
    if (atlas->cursor_y >= atlas->height) {
        return NULL;
    }
    if (atlas->cursor_x + dw >= atlas->width) {
        atlas->cursor_x = 0;
        atlas->cursor_y += font->height * font->density;
    }
    if (atlas->cursor_y + dh >= atlas->height) {
        // atlas is full
        atlas->cursor_y += dh;
        return NULL;
    }
    vl_byte_t *pixels = (atlas->data + atlas->width * atlas->cursor_y) + atlas->cursor_x;
    stbtt_MakeCodepointBitmap(&f->font, pixels, dw, dh, atlas->width, f->dense_scale, f->dense_scale, codepoint);
    float bx1 = atlas->cursor_x;
    float by1 = atlas->cursor_y;
    float bx2 = bx1 + dw;
    float by2 = by1 + dh;
    vl_font_atlas_codepoint_t result = {0};
    result.owner = font;
    result.w = dw;
    result.h = dh;
    result.codepoint = codepoint;
    result.advance_x = (advance_x * f->dense_scale);
    result.x = (left_bearing * f->dense_scale);
    result.y = (f->ascent * f->dense_scale) + dy1;
    float aw = atlas->width;
    float ah = atlas->height;
    result.uv.tl = VL_POINT(bx1 / atlas->width, by1 / atlas->height);
    result.uv.tr = VL_POINT(bx2 / atlas->width, by1 / atlas->height);
    result.uv.br = VL_POINT(bx2 / atlas->width, by2 / atlas->height);
    result.uv.bl = VL_POINT(bx1 / atlas->width, by2 / atlas->height);
    atlas->cursor_x += dw + font->density;
    return VL_DA_APPEND(atlas->codepoints, result);
}

int vl_font_universal_get_kern_advance(vl_font_t *font, uint32_t codepoint_a, uint32_t codepoint_b) {
    if (!font) return 0;
    vl_font_universal_t *f = (vl_font_universal_t*) font;
    return (int) (stbtt_GetGlyphKernAdvance(&f->font, codepoint_a, codepoint_b) * f->scale);
}

vl_result_t vl_font_universal_free(vl_font_t *font) {
    if (!font) return VL_ERROR;
    // nothing to free for us!
    return VL_SUCCESS;
}