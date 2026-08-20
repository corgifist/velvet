#ifndef VELVET_WEB_FONTS_H
#define VELVET_WEB_FONTS_H

#include "velvet/font/shaper.h"
#include "velvet/graphics/brush.h"
#include "velvet/font/atlas.h"
#include "velvet/graphics/bitmap.h"
#include "velvet/support/memory.h"
#include "velvet/font/font.h"

enum vl_web_font_weight {
    VL_WEB_FONT_THIN = 100,
    VL_WEB_FONT_EXTRA_LIGHT = 200,
    VL_WEB_FONT_LIGHT = 300,
    VL_WEB_FONT_REGULAR = 400,
    VL_WEB_FONT_MEDIUM = 500,
    VL_WEB_FONT_SEMI_BOLD = 600,
    VL_WEB_FONT_BOLD = 700,
    VL_WEB_FONT_EXTRA_BOLD = 800,
    VL_WEB_FONT_BLACK = 900,
    VL_WEB_FONT_EXTRA_BLACK = 950,
    VL_WEB_FONT_FORCE_INT = 2147483647,
    VL_WEB_FONT_FORCE_SIGNED = -2147483647
};

typedef enum vl_web_font_weight vl_web_font_weight_t;

struct vl_web_sized_font {
    vl_font_t *font;
    vl_font_shaper_font_ref_t *shaper_ref;
};

typedef struct vl_web_sized_font vl_web_sized_font_t;

struct vl_web_font {
    const vl_byte_t *font_data;
    size_t font_len;
    vl_web_font_weight_t weight; // 100, 200, 300 etc.
    vl_font_t *unit_font;
    vl_font_shaper_font_ref_t *unit_font_ref;
    VL_DA(vl_web_sized_font_t) sizes;
};

typedef struct vl_web_font vl_web_font_t;

struct vl_web_font_family {
    const char *name;
    VL_DA(vl_web_font_t) variations;
};

typedef struct vl_web_font_family vl_web_font_family_t;

struct vl_web_font_atlas {
    vl_font_atlas_t atlas;
    vl_graphics_bitmap_t *bitmap;
    vl_graphics_brush_t *brush;
};

typedef struct vl_web_font_atlas vl_web_font_atlas_t;

struct vl_web_font_atlas_codepoint {
    vl_web_font_atlas_t *atlas;
    vl_font_atlas_codepoint_t *codepoint;
};

typedef struct vl_web_font_atlas_codepoint vl_web_font_atlas_codepoint_t;

struct vl_web;
struct vl_web_fonts {
    struct vl_web *owner;

    VL_DA(vl_web_font_family_t) families;
    VL_DA(vl_web_font_atlas_t) atlases;

    vl_font_shaper_t *shaper;
};

typedef struct vl_web_fonts vl_web_fonts_t;

struct vl_web;
VL_API vl_result_t vl_web_fonts_init(vl_web_fonts_t *fonts, struct vl_web *owner);
VL_API vl_result_t vl_web_fonts_add_font(vl_web_fonts_t *fonts, const char *family_name, const vl_byte_t *font_data, size_t font_len, vl_web_font_weight_t weight);
VL_API vl_web_sized_font_t *vl_web_fonts_get_font(vl_web_fonts_t *fonts, const char *family_name, vl_web_font_weight_t weight, int height);
VL_API vl_web_sized_font_t *vl_web_fonts_get_font_by_unit_font(vl_web_fonts_t *fonts, vl_font_t *unit_font, vl_web_font_weight_t weight, int height);
VL_API vl_result_t vl_web_fonts_find_glyph_id(vl_web_fonts_t *fonts, vl_web_font_atlas_codepoint_t *codepoint, const char *family_name, vl_web_font_weight_t weight, int height, uint32_t glyph_id);
VL_API vl_result_t vl_web_fonts_find_glyph_id_with_font(vl_web_fonts_t *fonts, vl_web_font_atlas_codepoint_t *codepoint, vl_font_t *font, uint32_t glyph_id);
VL_API vl_web_sized_font_t *vl_web_fonts_find_sized_font_by_raw_font(vl_web_fonts_t *fonts, vl_font_t *raw_font);
VL_API vl_result_t vl_web_fonts_deinit(vl_web_fonts_t *fonts);

#endif // VELVET_WEB_FONTS_H