#ifndef VELVET_PLATFORM_UNIVERSAL_SHAPER_H
#define VELVET_PLATFORM_UNIVERSAL_SHAPER_H

#include "support/result.h"
#include "velvet/font/shaper.h"
#include "velvet/support/memory.h"
#include "velvet/platform/universal/kb_text_shape.h"

struct vl_font_shaper_universal {
    vl_font_shaper_t base;

    kbts_shape_context *context;
};

typedef struct vl_font_shaper_universal vl_font_shaper_universal_t;

struct vl_font_shaper_run_universal {
    vl_font_shaper_run_t base;

    kbts_run run;
    kbts_glyph *iterator;
};

typedef struct vl_font_shaper_run_universal vl_font_shaper_run_universal_t;

struct vl_font_shaper_font_ref_universal {
    vl_font_shaper_font_ref_t base;
    kbts_font font;
};

typedef struct vl_font_shaper_font_ref_universal vl_font_shaper_font_ref_universal_t;

vl_font_shaper_run_t *vl_font_shaper_run_universal_new(vl_font_shaper_t *shaper, vl_source_location_t loc);
vl_result_t vl_font_shaper_run_universal_reset(vl_font_shaper_run_t *shaper);
vl_result_t vl_font_shaper_run_universal_free(vl_font_shaper_run_t *run);

vl_font_shaper_t *vl_font_shaper_universal_new(vl_platform_context_t *context, vl_source_location_t loc);
vl_font_shaper_font_ref_t* vl_font_shaper_universal_add_font(vl_font_shaper_t *shaper, vl_font_t *font);
vl_result_t vl_font_shaper_universal_free_font(vl_font_shaper_t *shaper, vl_font_shaper_font_ref_t *font);
vl_result_t vl_font_shaper_univesal_process(vl_font_shaper_t *shaper, const char *text, size_t text_length);
bool vl_font_shaper_universal_shape(vl_font_shaper_t *shaper, vl_font_shaper_run_t *run);
vl_font_shaper_glyph_t *vl_font_shaper_universal_iterate(vl_font_shaper_run_t *run, vl_font_shaper_glyph_t *glyph);
vl_result_t vl_font_shaper_universal_free(vl_font_shaper_t *shaper);

#endif // VELVET_PLATFORM_UNIVERSAL_SHAPER_H