#ifndef VELVET_PLATFORM_UNIVERSAL_FONT_H
#define VELVET_PLATFORM_UNIVERSAL_FONT_H

#include "font/atlas.h"
#include "velvet/font/font.h"
#include "velvet/platform/universal/vl_truetype.h"

struct vl_font_universal_info {
    vl_font_info_t base;
    stbtt_fontinfo font;
    float scale, slim_scale;
};
typedef struct vl_font_universal_info vl_font_universal_info_t;

struct vl_font_universal {
    vl_font_t base;

    const vl_byte_t *data;
    size_t data_length;
};

typedef struct vl_font_universal vl_font_universal_t;

vl_font_t *vl_font_universal_new_with_subfont_indices(vl_platform_context_t *context, const char *name, int height, float density, const vl_byte_t *data, size_t data_length, VL_DA(int) subfonts, vl_source_location_t loc);
vl_font_t *vl_font_universal_new(vl_platform_context_t *context, const char *name, int height, float density, const vl_byte_t *data, size_t data_length, vl_source_location_t loc);
vl_font_atlas_codepoint_t *vl_font_universal_rasterize_glyph_id_with_font_index(vl_font_t *font, vl_font_atlas_t *atlas, uint32_t glyph_id, int font_index);
uint32_t vl_font_universal_get_glyph_id_and_font_index_by_codepoint(vl_font_t *font, uint32_t codepoint, int *font_index);
float vl_font_universal_get_kern_advance(vl_font_t *font, uint32_t codepoint_a, uint32_t codepoint_b);
vl_vec2_t vl_font_universal_get_text_size_ex(vl_font_t *font, const char *text, size_t text_length);
vl_result_t vl_font_universal_free(vl_font_t *font);

#endif // VELVET_PLATFORM_UNIVERSAL_FONT_H