#include "velvet/platform/context.h"
#include "support/allocator.h"
#include "support/da.h"
#include "velvet/support/feature.h"

#if VL_FEATURE(UNIVERSAL_PLATFORM)
    #include "velvet/platform/universal/window.h"
    #include "velvet/platform/universal/sleep.h"
    #include "velvet/platform/universal/render.h"
    #include "velvet/platform/universal/presentation.h"
    #include "velvet/platform/universal/brush.h"
    #include "velvet/platform/universal/bitmap.h"
#endif

static vl_result_t init_os_window(vl_platform_context_t *ctx, vl_platform_context_types_t *types) {
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    if (types->os_window == VL_PLATFORM_CONTEXT_UNIVERSAL) {
        ctx->os_window_new = vl_os_window_universal_new;
        ctx->os_window_poll_events = vl_os_window_universal_poll_events;
        ctx->os_window_should_close = vl_os_window_universal_should_close;
        ctx->os_window_free = vl_os_window_universal_free;
        return VL_SUCCESS;
    }
#endif

    // no backend found
    return VL_ERROR;
}

static vl_result_t init_os_sleep(vl_platform_context_t *ctx, vl_platform_context_types_t *types) {
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    if (types->os_sleep == VL_PLATFORM_CONTEXT_UNIVERSAL) {
        ctx->os_sleep = vl_os_sleep_universal;
        return VL_SUCCESS;
    }
#endif

    // no backend found
    return VL_ERROR;
}

static vl_result_t init_graphics(vl_platform_context_t *ctx, vl_platform_context_types_t *types) {
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    if (types->graphics_render == VL_PLATFORM_CONTEXT_UNIVERSAL) {
        ctx->graphics_render_new = vl_graphics_render_universal_new;
        ctx->graphics_render_clear = vl_graphics_render_universal_clear;
        ctx->graphics_render_batch_begin = vl_graphics_render_universal_batch_begin;
        ctx->graphics_render_batch_quad_colored_uv = vl_graphics_render_universal_batch_quad_colored_uv;
        ctx->graphics_render_batch_end = vl_graphics_render_universal_batch_end;
        ctx->graphics_render_resize = vl_graphics_render_universal_resize;
        ctx->graphics_render_free = vl_graphics_render_universal_free;

        ctx->graphics_bitmap_new = vl_graphics_bitmap_universal_new;
        ctx->graphics_bitmap_free = vl_graphics_bitmap_universal_free;

        ctx->graphics_brush_new_solid = vl_graphics_brush_universal_new_solid;
        ctx->graphics_brush_new_linear_gradient = vl_graphics_brush_universal_new_linear_gradient;
        ctx->graphics_brush_new_bitmap = vl_graphics_brush_universal_new_bitmap;
        ctx->graphics_brush_free = vl_graphics_brush_universal_free;

        return VL_SUCCESS;
    }
#endif

    // no backend found
    return VL_ERROR;
}

static vl_result_t init_graphics_presentation(vl_platform_context_t *ctx, vl_platform_context_types_t *types) {
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    if (types->graphics_presentation == VL_PLATFORM_CONTEXT_UNIVERSAL) {
        ctx->graphics_presentation_new = vl_graphics_presentation_universal_new;
        ctx->graphics_presentation_begin = vl_graphics_presentation_universal_begin;
        ctx->graphics_presentation_end = vl_graphics_presentation_universal_end;
        ctx->graphics_presentation_free = vl_graphics_presentation_universal_free;
        return VL_SUCCESS;
    }
#endif

    // no backend found
    return VL_ERROR;
}

static VL_DA(vl_platform_context_t*) s_contexts = NULL;

vl_platform_context_t *vl_platform_context_new_(vl_platform_context_types_t types, vl_source_location_t loc) {
    vl_platform_context_t *ctx = VL_NEW(vl_platform_context_t, loc);
    if (!ctx) return NULL;
    if (init_os_window(ctx, &types)) goto drop;
    if (init_os_sleep(ctx, &types)) goto drop;
    if (init_graphics(ctx, &types)) goto drop;
    if (init_graphics_presentation(ctx, &types)) goto drop;
    if (!s_contexts) s_contexts = VL_DA_INIT_WITH_ALLOCATOR(vl_platform_context_t*, VL_ALLOCATOR_STDLIB());
    VL_DA_APPEND(s_contexts, ctx);
    return ctx;

    drop:
    vl_free(ctx);
    return NULL;
}

vl_result_t vl_platform_context_free(vl_platform_context_t *context) {
    if (!context || !s_contexts || !vl_platform_context_valid(context)) return VL_ERROR;
    vl_free(context);
    for (int i = 0; i < VL_DA_LENGTH(s_contexts); i++) {
        if (s_contexts[i] == context) {
            VL_DA_DELETE(s_contexts, i);
            break;
        }
    }
    return VL_SUCCESS;
}

bool vl_platform_context_valid(vl_platform_context_t *context) {
    if (!context || !s_contexts) return false;
    for (int i = 0; i < VL_DA_LENGTH(s_contexts); i++) {
        if (s_contexts[i] == context) return true;
    }
    return false;
}