#include "velvet/graphics/render.h"
#include "graphics/brush.h"
#include "graphics/color.h"
#include "graphics/geometry.h"
#include "platform/universal/render.h"
#include "support/result.h"
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

vl_result_t vl_graphics_render_batch_quad_colored(vl_graphics_render_t *render, vl_quad_t quad, vl_graphics_brush_t *brush, vl_quad_colors_t colors) {
    if (!render) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_render_universal_batch_quad_colored(render, quad, brush, colors);
#endif 
    return VL_ERROR;
}

vl_result_t vl_graphics_render_batch_quad(vl_graphics_render_t *render, vl_quad_t quad, vl_graphics_brush_t *brush) {
    return vl_graphics_render_universal_batch_quad_colored(render, quad, brush, VL_QUAD_WHITE);
}

vl_result_t vl_graphics_render_batch_rect_colored(vl_graphics_render_t *render, vl_rect_t rect, vl_graphics_brush_t *brush, vl_quad_colors_t colors) {
    if (!render) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_render_universal_batch_rect_colored(render, rect, brush, colors);
#endif
    printf("no implementation for vl_graphics_render_rect\n");
    return VL_ERROR;
}

vl_result_t vl_graphics_render_batch_rect(vl_graphics_render_t *render, vl_rect_t rect, vl_graphics_brush_t *brush) {
    return vl_graphics_render_universal_batch_rect_colored(render, rect, brush, VL_QUAD_WHITE);
}

vl_result_t vl_graphics_render_batch_point(vl_graphics_render_t *render, vl_point_t point, int size, vl_color_t color) {
    return vl_graphics_render_batch_rect_colored(render, VL_RECT(
        VL_POINT(point.x - (float) size / 2, point.y - (float) size / 2),
        VL_POINT(point.x + (float) size / 2, point.y + (float) size / 2)
    ), NULL, VL_QUAD_COLORS(color, color, color, color));
}

vl_result_t vl_graphics_render_batch_end(vl_graphics_render_t *render) {
    if (!render) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_render_universal_batch_end(render);
#endif 
    return VL_ERROR;
}

vl_result_t vl_graphics_render_resize(vl_graphics_render_t *render, int w, int h) {
    if (!render) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_render_universal_resize(render, w, h);
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