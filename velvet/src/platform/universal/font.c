#include "font/atlas.h"
#include "support/base_math.h"
#include "support/error_pool.h"
#include "support/math.h"
#include "support/da.h"
#include "support/math.h"
#include <stddef.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "velvet/platform/universal/font.h"
#include "font/font.h"
#include "support/result.h"

vl_font_t *vl_font_universal_new_with_subfont_indices(vl_platform_context_t *context, const char *name, int height, float density, const vl_byte_t *data, size_t data_length, VL_DA(int) subfonts, vl_source_location_t loc) {
    if (!context) return NULL;
    vl_font_universal_t *font = VL_NEW(vl_font_universal_t, loc);
    if (!font) return NULL;
    font->base.name = name;
    font->base.context = context;
    font->base.height = height;
    font->base.density = density;

    font->data = data;
    font->data_length = data_length;

    int number_of_fonts = stbtt_GetNumberOfFonts(data);
    font->base.fonts = VL_DA_INIT(vl_font_info_t*, number_of_fonts);
    for (int i = 0; i < number_of_fonts; i++) {
        if (subfonts) {
            int len = VL_DA_LENGTH(subfonts);
            bool match = false;
            for (int j = 0; j < len; j++) {
                if (subfonts[j] == i) {
                    match = true;
                    break;
                }
            }
            if (!match) {
                *VL_DA_PUSH(font->base.fonts, vl_font_info_t*) = NULL;
                continue;
            }
        }
        vl_font_universal_info_t *font_info = VL_NEW(vl_font_universal_info_t);
        if (!stbtt_InitFont(&font_info->font, font->data, stbtt_GetFontOffsetForIndex(data, i))) {
            continue;
        }
        font_info->scale = stbtt_ScaleForMappingEmToPixels(&font_info->font, height * density);
        font_info->slim_scale = font_info->scale / density;
        int ascent, descent, line_gap;
        if (!stbtt_GetFontVMetricsOS2(&font_info->font, &ascent, &descent, &line_gap))
            stbtt_GetFontVMetrics(&font_info->font, &ascent, &descent, &line_gap);
        font_info->base.ascent = ascent * font_info->slim_scale;
        font_info->base.descent = descent * font_info->slim_scale;
        font_info->base.line_gap = VL_CEIL(line_gap * font_info->slim_scale);
        font_info->base.owner = (vl_font_t*) font;
        font_info->base.index = i;
        int len = 0;
        const char *face_name = NULL;
        for (int i = 0; i <= STBTT_UNICODE_EID_UNICODE_2_0_FULL; i++) {
            for (int j = 0; j <= STBTT_PLATFORM_ID_MICROSOFT; j++) {
                face_name = stbtt_GetFontNameString(&font_info->font, 
                &len, j, i, STBTT_MS_LANG_ENGLISH, 4
                );
                if (face_name && len > 0) goto found_name;
                face_name = stbtt_GetFontNameString(&font_info->font, 
                &len, j, i, STBTT_MAC_LANG_ENGLISH, 4
                );
                if (face_name && len > 0) goto found_name;
            }
        }
        found_name:
        if (face_name && len > 0) font_info->base.name = VL_STRING_INIT(face_name, len);
        VL_DA_APPEND(font->base.fonts, font_info);
    }
    return (vl_font_t*) font;
    err:
    vl_free(font);
    return NULL;
}

vl_font_atlas_codepoint_t *vl_font_universal_rasterize_glyph_id_with_font_index(vl_font_t *font, vl_font_atlas_t *atlas, uint32_t glyph_id, int font_index) {
    if (!font || !atlas) return NULL;
    if (atlas->format != VL_FONT_ATLAS_FORMAT_RRRR8) return NULL;
    if (atlas->full) return NULL;
    vl_font_universal_t *f = (vl_font_universal_t*) font;
    vl_font_universal_info_t *ui = (vl_font_universal_info_t*) f->base.fonts[font_index % VL_DA_LENGTH(f->base.fonts)];
    if (!ui) return NULL;
    int advance_x, left_bearing;
    stbtt_GetGlyphHMetrics(&ui->font, glyph_id, &advance_x, &left_bearing);
    int x1, y1, x2, y2;
    stbtt_GetGlyphBitmapBox(&ui->font, glyph_id, ui->scale, ui->scale, &x1, &y1, &x2, &y2);
    float w = x2 - x1;
    float h = y2 - y1;
    if (atlas->cursor_y >= atlas->height) {
        atlas->full = true;
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
    stbtt_MakeGlyphBitmapSubpixel(&ui->font, pixels, w, h, atlas->width, ui->scale, ui->scale, 0.0f, 0.0f, glyph_id);
    float bx1 = VL_FLOOR(atlas->cursor_x);
    float by1 = VL_FLOOR(atlas->cursor_y);
    float bx2 = VL_FLOOR(bx1 + w);
    float by2 = VL_FLOOR(by1 + h);
    vl_font_atlas_codepoint_t result = {0};
    result.owner = font;
    
    float lb = left_bearing * ui->slim_scale;
    float ax = advance_x * ui->slim_scale;
    result.advance_x = ax;
    result.x1 = x1 - lb;
    result.y1 = ui->base.ascent + ui->base.line_gap + ((float) y1) / font->density;
    result.x2 = x2 / font->density - lb;
    result.y2 = ui->base.ascent + ((float) y2) / font->density;
    result.w = w / font->density;
    result.h = h / font->density;
    result.glyph_id = glyph_id;

    float aw = atlas->width;
    float ah = atlas->height;
    result.uv.tl = VL_POINT(bx1 / aw, by1 / ah);
    result.uv.tr = VL_POINT(bx2 / aw, by1 / ah);
    result.uv.br = VL_POINT(bx2 / aw, by2 / ah);
    result.uv.bl = VL_POINT(bx1 / aw, by2 / ah);
    atlas->cursor_x += w + 2;
    atlas->largest_glyph_on_line = VL_MAX(h, atlas->largest_glyph_on_line);
    result.index = VL_DA_LENGTH(atlas->codepoints);
    result.font_index = font_index;
    return VL_DA_APPEND(atlas->codepoints, result);
}

uint32_t vl_font_universal_get_glyph_id_and_font_index_by_codepoint(vl_font_t *font, uint32_t codepoint, int *font_index) {
    if (!font) return 0;
    vl_font_universal_t *f = (vl_font_universal_t*) font;
    for (int i = 0; i < VL_DA_LENGTH(font->fonts); i++) {
        vl_font_universal_info_t *ui = (vl_font_universal_info_t*) font->fonts[i];
        if (!ui) continue;
        uint32_t glyph_id = stbtt_FindGlyphIndex(&ui->font, codepoint);
        if (glyph_id != 0) {
            if (font_index) *font_index = i;
            return glyph_id;
        }
    }
    if (font_index) *font_index = 0;
    return 0;
}

float vl_font_universal_get_kern_advance(vl_font_t *font, uint32_t codepoint_a, uint32_t codepoint_b) {
    if (codepoint_a == 0 || codepoint_b == 0) return 0;
    vl_font_universal_t *f = (vl_font_universal_t*) font;
    // return ((float) stbtt_GetGlyphKernAdvance(&f->font, codepoint_a, codepoint_b)) * f->slim_scale;
    return 0;
}

vl_vec2_t vl_font_universal_get_text_size_ex(vl_font_t *font, const char *text, size_t text_length) {
    vl_font_universal_t *f = (vl_font_universal_t*) font;
    float x = 0;
    float y = 0;
    float base_x = 0;
    float base_y = 0;
    // for (size_t i = 0; i < text_length; i++) {
    //     int c = text[i];
    //     if (c == '\n') {
    //         base_y += font->newline_advance;
    //         base_x = 0;
    //         y = VL_MAX(y, base_y);
    //         continue;
    //     }
    //     int ax, lsb;
    //     stbtt_GetCodepointHMetrics(&f->font, c, &ax, &lsb);
    //     int x1, y1, x2, y2;
    //     stbtt_GetCodepointBitmapBox(&f->font, c, f->slim_scale, f->slim_scale, &x1, &y1, &x2, &y2);
    //     int w = x2 - x1;
    //     int h = y2 - y1;
    //     base_x += ax * f->slim_scale;
    //     float bx = base_x + (lsb + w) * f->slim_scale;
    //     float by = base_y + f->base.ascent - f->base.descent + f->base.line_gap + (y1 + h) / font->density;
    //     x = VL_MAX(bx, x);
    //     y = VL_MAX(by, y);
    //     if (i != text_length - 1)
    //         base_x += stbtt_GetGlyphKernAdvance(&f->font, c, text[i + 1]) * f->slim_scale;
    // }
    return VL_VEC2(x, y);
}

vl_result_t vl_font_universal_free(vl_font_t *font) {
    if (!font) return VL_ERROR;
    for (int i = 0; i < VL_DA_LENGTH(font->fonts); i++) {
        vl_font_info_t *info = font->fonts[i];
        if (info) {
            VL_STRING_FREE(info->name);
        }
        vl_free(font->fonts[i]);
    }
    VL_DA_FREE(font->fonts);
    return VL_SUCCESS;
}