#ifndef VELVET_FONT_SHAPER_H
#define VELVET_FONT_SHAPER_H

#include "velvet/support/memory.h"
#include "velvet/support/result.h"
#include "velvet/platform/context.h"
#include "velvet/font/font.h"

struct vl_font_shaper {
    vl_platform_context_t *context;
};

typedef struct vl_font_shaper vl_font_shaper_t;

struct vl_font_shaper_glyph {
    uint32_t codepoint;
    float x, y;
    float advance_x, advance_y;
};

typedef struct vl_font_shaper_glyph vl_font_shaper_glyph_t;

struct vl_font_shaper_run {
    vl_font_shaper_t *owner;
    vl_font_t *font;
    bool newline;
};

typedef struct vl_font_shaper_run vl_font_shaper_run_t;

#define vl_font_shaper_run_new_va_expand(shaper, loc) \
    vl_font_shaper_run_new_(shaper, loc)
#define vl_font_shaper_run_new(...) \
    vl_font_shaper_run_new_va_expand(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)
VL_API vl_font_shaper_run_t *vl_font_shaper_run_new_(vl_font_shaper_t *shaper, vl_source_location_t loc);
VL_API vl_result_t vl_font_shaper_run_reset(vl_font_shaper_run_t *run);
VL_API vl_result_t vl_font_shaper_run_free(vl_font_shaper_run_t *run);

#define vl_font_shaper_va_expand(context, loc) \
    vl_font_shaper_new_(context, loc)
#define vl_font_shaper_new(...) \
    vl_font_shaper_va_expand(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)
VL_API vl_font_shaper_t *vl_font_shaper_new_(vl_platform_context_t *context, vl_source_location_t loc);
VL_API vl_result_t vl_font_shaper_add_font(vl_font_shaper_t *shaper, vl_font_t *font);
VL_API vl_result_t vl_font_shaper_process(vl_font_shaper_t *shaper, const char *text, size_t text_length);
VL_API bool vl_font_shaper_shape(vl_font_shaper_t *shaper, vl_font_shaper_run_t *run);
VL_API vl_font_shaper_glyph_t *vl_font_shaper_iterate(vl_font_shaper_run_t *run, vl_font_shaper_glyph_t *glyph);
VL_API vl_result_t vl_font_shaper_free(vl_font_shaper_t *shaper);

#endif // VELVET_FONT_SHAPER