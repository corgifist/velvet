#ifndef VELVET_FONT_FONT_H
#define VELVET_FONT_FONT_H

#include "velvet/common.h"
#include "velvet/support/memory.h"
#include "velvet/support/api.h"
#include "velvet/support/result.h"
#include "velvet/platform/context.h"
#include "velvet/font/atlas.h"

struct vl_font {
    vl_platform_context_t *context;
    const char *name;
    int height;
    float newline_advance;
    float density;
    int ascent, descent, line_gap;
};

typedef struct vl_font vl_font_t;

#define vl_font_new_va_expand(context, name, height, density, data, data_length, loc, ...) \
    vl_font_new_(context, name, height, density, (const vl_byte_t*) data, data_length, loc)
#define vl_font_new(...) \
    vl_font_new_va_expand(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)
VL_API vl_font_t *vl_font_new_(vl_platform_context_t *context, const char *name, int height, float density, const vl_byte_t *data, size_t data_length, vl_source_location_t loc);
VL_API vl_font_atlas_codepoint_t *vl_font_rasterize_codepoint(vl_font_t *font, vl_font_atlas_t *atlas, uint32_t codepoint);
VL_API vl_font_atlas_codepoint_t *vl_font_rasterize_glyph_id(vl_font_t *font, vl_font_atlas_t *atlas, uint32_t glyph_id);
VL_API vl_result_t vl_font_rasterize_codepoint_range(vl_font_t *font, vl_font_atlas_t *atlas, uint32_t begin, uint32_t end);
VL_API vl_result_t vl_font_rasterize_codepoints(vl_font_t *font, vl_font_atlas_t *atlas, const char *codepoints);
VL_API uint32_t vl_font_get_glyph_id_by_codepoint(vl_font_t *font, uint32_t codepoint);
VL_API float vl_font_get_kern_advance(vl_font_t *font, uint32_t codepoint_a, uint32_t codepoint_b);
VL_API vl_vec2_t vl_font_get_text_size(vl_font_t *font, const char *text);
VL_API vl_vec2_t vl_font_get_text_size_ex(vl_font_t *font, const char *text, size_t text_length);
VL_API vl_result_t vl_font_free(vl_font_t *font);

#endif // VELVET_FONT_FONT_H