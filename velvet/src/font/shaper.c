#include "font/shaper.h"
#include "platform/context.h"
#include "support/memory.h"
#include "support/result.h"

VL_API vl_font_shaper_run_t *vl_font_shaper_run_new_(vl_font_shaper_t *shaper, vl_source_location_t loc) {
    if (!shaper || !shaper->context || !shaper->context->font_shaper_run_new) return NULL;
    return shaper->context->font_shaper_run_new(shaper, loc);
}

vl_font_shaper_t *vl_font_shaper_new_(vl_platform_context_t *context, vl_source_location_t loc) {
    if (!context || !context->font_shaper_new) return NULL;
    return context->font_shaper_new(context, loc);
}

vl_font_shaper_font_ref_t *vl_font_shaper_add_font(vl_font_shaper_t *shaper, vl_font_t *font) {
    if (!shaper || !font || !shaper->context || !shaper->context->font_shaper_add_font) return NULL;
    return shaper->context->font_shaper_add_font(shaper, font);
}

vl_result_t vl_font_shaper_free_font(vl_font_shaper_t *shaper, vl_font_shaper_font_ref_t *font) {
    if (!shaper || !font || !shaper->context || !shaper->context->font_shaper_free_font) return VL_ERROR;
    return shaper->context->font_shaper_free_font(shaper, font);
}

vl_result_t vl_font_shaper_process(vl_font_shaper_t *shaper, const char *text, size_t text_length) {
    if (!shaper || !text || !shaper->context || !shaper->context->font_shaper_process) return VL_ERROR;
    return shaper->context->font_shaper_process(shaper, text, text_length);
}

bool vl_font_shaper_shape(vl_font_shaper_t *shaper, vl_font_shaper_run_t *run) {
    if (!shaper || !run || !shaper->context || !shaper->context->font_shaper_shape) return false;
    return shaper->context->font_shaper_shape(shaper, run);
}

vl_font_shaper_glyph_t *vl_font_shaper_iterate(vl_font_shaper_run_t *run, vl_font_shaper_glyph_t *glyph) {
    if (!glyph || !run || !run->owner->context || !run->owner->context->font_shaper_iterate) return NULL;
    return run->owner->context->font_shaper_iterate(run, glyph);
}

vl_result_t vl_font_shaper_run_reset(vl_font_shaper_run_t *run) {
    if (!run || !run->font || !run->font->context || !run->font->context->font_shaper_run_reset) return VL_ERROR;
    return run->font->context->font_shaper_run_reset(run);
}

vl_result_t vl_font_shaper_run_free(vl_font_shaper_run_t *run) {
    if (!run || !run->owner || !run->owner->context || !run->owner->context->font_shaper_run_free) return VL_ERROR;
    return run->owner->context->font_shaper_run_free(run);
}

vl_result_t vl_font_shaper_free(vl_font_shaper_t *shaper) {
    if (!shaper || !shaper->context || !shaper->context->font_shaper_free) return VL_ERROR;
    return shaper->context->font_shaper_free(shaper);
}