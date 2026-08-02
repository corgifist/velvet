#include "platform/universal/kb_text_shape.h"
#define KB_TEXT_SHAPE_IMPLEMENTATION
#include "velvet/font/shaper.h"
#include "platform/context.h"
#include "platform/universal/shaper.h"
#include "support/memory.h"
#include "support/result.h"
#include "platform/universal/font.h"

vl_font_shaper_run_t *vl_font_shaper_run_universal_new(vl_font_shaper_t *shaper, vl_source_location_t loc) {
    vl_font_shaper_run_universal_t *run = VL_NEW(vl_font_shaper_run_universal_t, loc);
    if (!run) return NULL;
    run->base.owner = shaper;
    run->base.font = NULL;
    run->base.newline = false;
    run->run = (kbts_run) {0};
    run->iterator = NULL;
    return (vl_font_shaper_run_t*) run;
}

vl_result_t vl_font_shaper_run_universal_free(vl_font_shaper_run_t *run) {
    vl_free(run);
    return VL_SUCCESS;
}

vl_font_shaper_t *vl_font_shaper_universal_new(vl_platform_context_t *context, vl_source_location_t loc) {
    vl_font_shaper_universal_t *shaper = VL_NEW(vl_font_shaper_universal_t, loc);
    if (!shaper) goto err;
    shaper->base.context = context;
    shaper->context = kbts_CreateShapeContext(NULL, NULL);
    if (!shaper->context) goto err;

    return (vl_font_shaper_t*) shaper;
    err:
    vl_free(shaper);
    return NULL;
}


vl_result_t vl_font_shaper_universal_add_font(vl_font_shaper_t *shaper, vl_font_t *font) {
    vl_font_shaper_universal_t *s = (vl_font_shaper_universal_t*) shaper;
    vl_font_universal_t *f = (vl_font_universal_t*) font;
    kbts_font *added_font = kbts_ShapePushFontFromMemory(s->context, (void*) f->data, f->data_length, 0);
    if (!added_font) return VL_ERROR;
    added_font->UserData = font;
    return VL_SUCCESS;
}

vl_result_t vl_font_shaper_univesal_process(vl_font_shaper_t *shaper, const char *text, size_t text_length) {
    vl_font_shaper_universal_t *s = (vl_font_shaper_universal_t*) shaper;
    kbts_ShapeBegin(s->context, KBTS_DIRECTION_DONT_KNOW, KBTS_LANGUAGE_DONT_KNOW);
    kbts_ShapeUtf8(s->context, text, text_length, KBTS_USER_ID_GENERATION_MODE_CODEPOINT_INDEX);
    kbts_ShapeEnd(s->context);
    return VL_SUCCESS;
}

bool vl_font_shaper_universal_shape(vl_font_shaper_t *shaper, vl_font_shaper_run_t *run) {
    vl_font_shaper_universal_t *s = (vl_font_shaper_universal_t*) shaper;
    vl_font_shaper_run_universal_t *r = (vl_font_shaper_run_universal_t*) run;
    kbts_shape_error err = kbts_ShapeError(s->context);
    bool status = kbts_ShapeRun(s->context, &r->run);
    if (!status) return false;
    run->newline = r->run.Flags & KBTS_BREAK_FLAG_LINE_HARD;
    run->font = r->run.Font->UserData;
    return true;
}

vl_font_shaper_glyph_t *vl_font_shaper_universal_iterate(vl_font_shaper_run_t *run, vl_font_shaper_glyph_t *glyph) {
    vl_font_universal_t *f = (vl_font_universal_t*) run->font;
    vl_font_shaper_run_universal_t *r = (vl_font_shaper_run_universal_t*) run;
    int status = kbts_GlyphIteratorNext(&r->run.Glyphs, &r->iterator);
    if (!status) return NULL;
    glyph->codepoint = r->iterator->Codepoint;
    glyph->codepoint_index = r->iterator->UserIdOrCodepointIndex;
    glyph->x = ((float) r->iterator->OffsetX) * f->slim_scale;
    glyph->y = ((float) r->iterator->OffsetY) * f->slim_scale;
    glyph->advance_x = ((float) r->iterator->AdvanceX) * f->slim_scale;
    glyph->advance_y = ((float) r->iterator->AdvanceY) * f->slim_scale;
    glyph->id = r->iterator->Id;
    return glyph;
}

vl_result_t vl_font_shaper_run_universal_reset(vl_font_shaper_run_t *run) {
    vl_font_shaper_run_universal_t *r = (vl_font_shaper_run_universal_t*) run;
    r->iterator = NULL;
    r->run = (kbts_run) {0};
    return VL_SUCCESS;
}

vl_result_t vl_font_shaper_universal_free(vl_font_shaper_t *shaper) {
    if (!shaper) return VL_ERROR;
    vl_font_shaper_universal_t *s = (vl_font_shaper_universal_t*) shaper;
    kbts_DestroyShapeContext(s->context);
    vl_free(shaper);
    return VL_SUCCESS;
}