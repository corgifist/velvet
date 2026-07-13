#include "velvet/graphics/render.h"
#include "graphics/brush.h"
#include "platform/universal/render.h"
#include "velvet/support/feature.h"

vl_graphics_render_t *vl_graphics_render_new(vl_os_window_t *window) {
    if (!window) return NULL;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_render_universal_new(window);
#endif // VL_FEATURE(UNIVERSAL_PLATFORM)
    printf("no implementation for vl_graphics_render_new\n");
    return NULL;
}

vl_result_t vl_graphics_render_clear(vl_graphics_render_t *render, vl_color_t fill) {
    if (!render) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_render_universal_clear(render, fill);
#endif
    printf("no implementation for vl_graphics_render_fill\n");
    return VL_ERROR;
}

vl_result_t vl_graphics_render_batch_begin(vl_graphics_render_t *render) {
    if (!render) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_render_universal_batch_begin(render);
#endif 
    return VL_ERROR;
}

vl_result_t vl_graphics_render_batch_rect(vl_graphics_render_t *render, vl_rect_t rect, vl_graphics_brush_t *brush) {
    if (!render) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_render_universal_batch_rect(render, rect, brush);
#endif
    printf("no implementation for vl_graphics_render_rect\n");
    return VL_ERROR;
}

vl_result_t vl_graphics_render_batch_end(vl_graphics_render_t *render) {
    if (!render) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_render_universal_batch_end(render);
#endif 
    return VL_ERROR;
}

vl_result_t vl_graphics_render_free(vl_graphics_render_t *render) {
    if (!render) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_render_universal_free(render);
#endif // VL_FEATURE(UNIVERSAL_PLATFORM)
    printf("no implementation for vl_graphics_render_free\n");
    return VL_ERROR;
}