#ifndef VELVET_PLATFORM_UNIVERSAL_FONT_H
#define VELVET_PLATFORM_UNIVERSAL_FONT_H

#include "font/atlas.h"
#include "velvet/font/font.h"
#include "velvet/platform/universal/stb_truetype.h"

struct vl_font_universal {
    vl_font_t base;

    const vl_byte_t *data;
    size_t data_length;
    stbtt_fontinfo font;
    float scale, slim_scale;
    int ascent, descent, line_gap;
};

typedef struct vl_font_universal vl_font_universal_t;

vl_font_t *vl_font_universal_new(vl_platform_context_t *context, const char *name, int height, float density, const vl_byte_t *data, size_t data_length, vl_source_location_t loc);
vl_font_atlas_codepoint_t *vl_font_universal_rasterize_glyph_id(vl_font_t *font, vl_font_atlas_t *atlas, uint32_t glyph_id);
uint32_t vl_font_universal_get_glyph_id_by_codepoint(vl_font_t *font, uint32_t codepoint);
float vl_font_universal_get_kern_advance(vl_font_t *font, uint32_t codepoint_a, uint32_t codepoint_b);
vl_vec2_t vl_font_universal_get_text_size_ex(vl_font_t *font, const char *text, size_t text_length);
vl_result_t vl_font_universal_free(vl_font_t *font);

#endif // VELVET_PLATFORM_UNIVERSAL_FONT_H