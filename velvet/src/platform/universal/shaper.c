#include "platform/universal/vl_text_shape.h"
#include "support/da.h"
#include "support/global_error_pool.h"
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
    return (vl_font_shaper_run_t*) run;
}

vl_result_t vl_font_shaper_run_universal_free(vl_font_shaper_run_t *run) {
    vl_free(run);
    return VL_SUCCESS;
}

static void vl_kbts_allocator(void *Data, kbts_allocator_op *Op) {
    switch(Op->Kind) {
    case KBTS_ALLOCATOR_OP_KIND_ALLOCATE:
        Op->Allocate.Pointer = vl_malloc(Op->Allocate.Size);
        break;
    case KBTS_ALLOCATOR_OP_KIND_FREE:
        vl_free(Op->Free.Pointer);
        break;
    }
}

vl_font_shaper_t *vl_font_shaper_universal_new(vl_platform_context_t *context, vl_source_location_t loc) {
    vl_font_shaper_universal_t *shaper = VL_NEW(vl_font_shaper_universal_t, loc);
    if (!shaper) return NULL;
    shaper->base.context = context;
    shaper->context = kbts_CreateShapeContext(vl_kbts_allocator, NULL);
    shaper->base.font_stack = VL_DA_INIT(vl_font_shaper_font_ref_t*);
    if (!shaper->context) goto err;
    return (vl_font_shaper_t*) shaper;
err:
    VL_DA_FREE(shaper->base.font_stack);
    vl_free(shaper);
    return NULL;
}

vl_font_shaper_font_ref_t *vl_font_shaper_universal_add_font(vl_font_shaper_t *shaper, vl_font_t *font) {
    if (!shaper || !font) return NULL;
    vl_font_universal_t *f = (vl_font_universal_t*) font;
    vl_font_shaper_font_ref_universal_t *font_ref = VL_NEW(vl_font_shaper_font_ref_universal_t);
    font_ref->base.font = font;
    font_ref->fonts = VL_DA_INIT(kbts_font);
    for (int i = 0; i < VL_DA_LENGTH(font->fonts); i++) {
        vl_font_info_t *fi = font->fonts[i];
        if (!fi) continue;
        kbts_font kb_font = kbts_FontFromMemory((void*) f->data, (int)f->data_length, (int)i, vl_kbts_allocator, NULL);
        if (kb_font.Error != KBTS_LOAD_FONT_ERROR_NONE) {
            kbts_FreeFont(&kb_font);
            goto error;
        }
        kb_font.UserData = fi;
        VL_DA_APPEND(font_ref->fonts, kb_font);
    }
    VL_DA_APPEND(shaper->font_stack, font_ref);
    return (vl_font_shaper_font_ref_t*) font_ref;

    error:
    for (int i = 0; i < VL_DA_LENGTH(font_ref->fonts); i++)
        kbts_FreeFont(font_ref->fonts + i);
    VL_DA_FREE(font_ref->fonts);
    vl_free(font_ref);
    return NULL;
}

vl_result_t vl_font_shaper_universal_free_font(vl_font_shaper_t *shaper, vl_font_shaper_font_ref_t *font) {
    if (!font) return VL_ERROR;
    vl_font_shaper_font_ref_universal_t *f = (vl_font_shaper_font_ref_universal_t*) font;
    for (size_t i = 0; i < VL_DA_LENGTH(f->fonts); i++) kbts_FreeFont(f->fonts + i);
    VL_DA_FREE(f->fonts);
    vl_free(font);
    return VL_SUCCESS;
}

static vl_result_t shaper_process(vl_font_shaper_t *shaper, bool is_spans,
                                  const void *data, int arg1, int arg2) {
    vl_font_shaper_universal_t *s = (vl_font_shaper_universal_t*) shaper;
    if (!shaper || !s->context || (!data && arg1 != 0)) return VL_ERROR;
    if (!shaper->font_stack || VL_DA_LENGTH(shaper->font_stack) == 0) {
        vl_global_error_pool_append("font stack is either empty or NULL for vl_font_shaper_t %p", shaper);
        return VL_ERROR;
    }

    int fonts_count = 0;
    for (size_t i = 0; i < VL_DA_LENGTH(shaper->font_stack); i++) {
        vl_font_shaper_font_ref_universal_t *uf = (vl_font_shaper_font_ref_universal_t*) shaper->font_stack[i];
        if (!uf) continue;
        for (size_t j = 0; j < VL_DA_LENGTH(uf->fonts); j++) {
            kbts_ShapePushFont(s->context, uf->fonts + j);
            fonts_count++;
        }
    }

    vl_result_t result = VL_SUCCESS;
    if (!is_spans) {
        const char *text = (const char*)data;
        kbts_ShapeBegin(s->context, KBTS_DIRECTION_DONT_KNOW, KBTS_LANGUAGE_DONT_KNOW);
        kbts_ShapeUtf8(s->context, text, (int)arg1, KBTS_USER_ID_GENERATION_MODE_CODEPOINT_INDEX);
        kbts_ShapeEnd(s->context);
        if (kbts_ShapeError(s->context) != KBTS_SHAPE_ERROR_NONE) result = VL_ERROR;
    } else {
        const VL_DA(vl_font_shaper_span_t) spans = (const vl_font_shaper_span_t*)data;
        kbts_utf8_span *kb_spans = VL_DA_INIT(kbts_utf8_span, arg1);
        for (int i = 0; i < arg1; i++) {
            kbts_utf8_span kb_span = (kbts_utf8_span) {
                    spans[i].text, 
                    spans[i].len
            };
            VL_DA_APPEND(kb_spans, kb_span);
        }
        kbts_ShapeUtf8SpansFrom(s->context, KBTS_DIRECTION_DONT_KNOW, KBTS_LANGUAGE_DONT_KNOW, kb_spans, (int)arg1, (int)arg2);
        VL_DA_FREE(kb_spans);
    }

    for (int i = 0; i < fonts_count; i++) kbts_ShapePopFont(s->context);
    return result;
}

vl_result_t vl_font_shaper_universal_process(vl_font_shaper_t *shaper, const char *text, int text_length) {
    return shaper_process(shaper, false, text, text_length, 0);
}

vl_result_t vl_font_shaper_universal_process_spans(vl_font_shaper_t *shaper,
                                                    const vl_font_shaper_span_t *spans,
                                                    int spans_count, int first_span_index) {
    if (spans_count < 0 || (spans_count && !spans)) return VL_ERROR;
    if (first_span_index < 0) first_span_index = 0;
    if (first_span_index > spans_count) return VL_ERROR;
    return shaper_process(shaper, true, spans, spans_count, first_span_index);
}

bool vl_font_shaper_universal_shape(vl_font_shaper_t *shaper, vl_font_shaper_run_t *run) {
    if (!shaper || !run) return false;
    vl_font_shaper_universal_t *s = (vl_font_shaper_universal_t*) shaper;
    vl_font_shaper_run_universal_t *r = (vl_font_shaper_run_universal_t*) run;
    if (!shaper->font_stack || VL_DA_LENGTH(shaper->font_stack) <= 0) return false;
    bool status = kbts_ShapeRun(s->context, &r->run);
    if (!status) return false;
    run->hard_line_break = (r->run.Flags & KBTS_BREAK_FLAG_LINE_HARD) != 0;
    run->span_index = r->run.SpanIndex;
    run->font = r->run.Font ? r->run.Font->UserData : NULL;
    switch (r->run.ParagraphDirection) {
    case KBTS_DIRECTION_RTL: run->paragraph_direction = VL_FONT_SHAPER_RUN_DIRECTION_RTL; break;
    case KBTS_DIRECTION_LTR: run->paragraph_direction = VL_FONT_SHAPER_RUN_DIRECTION_LTR; break;
    default: run->paragraph_direction = VL_FONT_SHAPER_RUN_DIRECTION_NONE; break;
    }
    switch (r->run.Direction) {
    case KBTS_DIRECTION_LTR: run->direction = VL_FONT_SHAPER_RUN_DIRECTION_LTR; break;
    case KBTS_DIRECTION_RTL: run->direction = VL_FONT_SHAPER_RUN_DIRECTION_RTL; break;
    default: run->direction = VL_FONT_SHAPER_RUN_DIRECTION_NONE; break;
    }
    return true;
}

vl_font_shaper_glyph_t *vl_font_shaper_universal_iterate(vl_font_shaper_run_t *run, vl_font_shaper_glyph_t *glyph) {
    if (!run || !glyph || !run->owner) return NULL;
    vl_font_shaper_universal_t *s = (vl_font_shaper_universal_t*)run->owner;
    vl_font_shaper_run_universal_t *r = (vl_font_shaper_run_universal_t*)run;
    vl_font_universal_info_t *f = (vl_font_universal_info_t*)run->font;
    if (!f) return NULL;
    int status = kbts_GlyphIteratorNext(&r->run.Glyphs, &r->iterator);
    if (!status) return NULL;
    glyph->codepoint = (uint32_t)r->iterator->Codepoint;
    glyph->codepoint_index = r->iterator->UserIdOrCodepointIndex;
    glyph->x = ((float) r->iterator->OffsetX) * f->slim_scale;
    glyph->y = ((float) r->iterator->OffsetY) * f->slim_scale;
    glyph->advance_x = ((float) r->iterator->AdvanceX) * f->slim_scale;
    glyph->advance_y = ((float) r->iterator->AdvanceY) * f->slim_scale;
    glyph->id = r->iterator->Id;
    glyph->last = r->iterator->IsLastInRun != 0;
    glyph->span_index = r->iterator->SpanIndex;
    glyph->span_cluster_first = r->iterator->SpanClusterFirst;
    glyph->span_cluster_last = r->iterator->SpanClusterLast;
    glyph->source_cluster_first = r->iterator->SourceClusterFirst;
    glyph->source_cluster_last = r->iterator->SourceClusterLast;
    return glyph;
}

vl_result_t vl_font_shaper_run_universal_reset(vl_font_shaper_run_t *run) {
    if (!run) return VL_ERROR;
    vl_font_shaper_run_universal_t *r = (vl_font_shaper_run_universal_t*)run;
    vl_font_shaper_t *owner = r->base.owner;
    VL_ZERO_OUT(r);
    r->base.owner = owner;
    return VL_SUCCESS;
}

vl_result_t vl_font_shaper_universal_free(vl_font_shaper_t *shaper) {
    if (!shaper) return VL_ERROR;
    vl_font_shaper_universal_t *s = (vl_font_shaper_universal_t*)shaper;
    kbts_DestroyShapeContext(s->context);
    VL_DA_FREE(shaper->font_stack);
    vl_free(shaper);
    return VL_SUCCESS;
}