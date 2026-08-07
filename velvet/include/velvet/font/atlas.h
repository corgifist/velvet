#ifndef VELVET_FONT_ATLAS_H
#define VELVET_FONT_ATLAS_H

#include "velvet/support/da.h"
#include "velvet/support/api.h"
#include "velvet/support/result.h"
#include "velvet/support/memory.h"
#include "velvet/graphics/geometry.h"

enum vl_font_atlas_format {
    VL_FONT_ATLAS_FORMAT_RRRR8 = 1
};

typedef enum vl_font_atlas_format vl_font_atlas_format_t;


struct vl_font;
struct vl_font_atlas_codepoint {
    struct vl_font *owner;
    uint32_t glyph_id;
    vl_quad_uv_t uv;

    // Data for basic text shaping
    // Consider using vl_font_shaper for better-looking text shaping
    float w, h;
    float x1, y1;
    float x2, y2;
    float advance_x;
};

typedef struct vl_font_atlas_codepoint vl_font_atlas_codepoint_t;

struct vl_font_atlas {
    vl_font_atlas_format_t format;
    size_t width, height;
    vl_byte_t *data;
    VL_DA(vl_font_atlas_codepoint_t) codepoints;
    size_t cursor_x, cursor_y;
    size_t largest_glyph_on_line;
    bool full;
};

typedef struct vl_font_atlas vl_font_atlas_t;

#define vl_font_atlas_new_va_expand(format, width, height, loc, ...) \
    vl_font_atlas_new_(format, width, height, loc)
#define vl_font_atlas_new(...) \
    vl_font_atlas_new_va_expand(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)
VL_API vl_font_atlas_t *vl_font_atlas_new_(vl_font_atlas_format_t format, size_t width, size_t height, vl_source_location_t loc);

#define vl_font_atlas_init_va_expand(atlas, format, width, height, loc, ...) \
    vl_font_atlas_init_(atlas, format, width, height, loc)
#define vl_font_atlas_init(...) \
    vl_font_atlas_init_va_expand(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)
VL_API vl_result_t vl_font_atlas_init_(vl_font_atlas_t *atlas, vl_font_atlas_format_t format, size_t width, size_t height, vl_source_location_t loc);
VL_API vl_font_atlas_codepoint_t *vl_font_atlas_find_glyph_id(vl_font_atlas_t *atlas, struct vl_font *font, uint32_t glyph_id);
VL_API vl_result_t vl_font_atlas_deinit(vl_font_atlas_t *atlas);
VL_API vl_result_t vl_font_atlas_free(vl_font_atlas_t *atlas);

#endif // VELVET_FONT_ATLAS_H