#include "velvet/graphics/render.h"
#include "graphics/brush.h"
#include "graphics/color.h"
#include "graphics/geometry.h"
#include "platform/context.h"
#include "support/result.h"
#include "velvet/support/feature.h"

vl_graphics_render_t *vl_graphics_render_new(vl_os_window_t *window) {
    if (!window || !vl_platform_context_valid(window->context) || !window->context->graphics_render_new) return NULL;
    vl_graphics_render_t *render = window->context->graphics_render_new(window);
    if (render) {
        render->context = window->context;
    }
    return render;
}

vl_result_t vl_graphics_render_clear(vl_graphics_render_t *render, vl_color_t fill) {
    if (!render || !vl_platform_context_valid(render->context) || !render->context->graphics_render_clear) return VL_ERROR;
    return render->context->graphics_render_clear(render, fill);
}

vl_result_t vl_graphics_render_batch_begin(vl_graphics_render_t *render) {
    if (!render || !vl_platform_context_valid(render->context) || !render->context->graphics_render_batch_begin) return VL_ERROR;
    return render->context->graphics_render_batch_begin(render);
}

vl_result_t vl_graphics_render_batch_quad_colored(vl_graphics_render_t *render, vl_quad_t quad, vl_graphics_brush_t *brush, vl_quad_colors_t colors) {
    return vl_graphics_render_batch_quad_colored_uv(render, quad, brush, colors, VL_QUAD_UV_DEFAULT);
}


vl_result_t vl_graphics_render_batch_quad_colored_uv(vl_graphics_render_t *render, vl_quad_t quad, vl_graphics_brush_t *brush, vl_quad_colors_t colors, vl_quad_uv_t uv) {
    if (!render || !vl_platform_context_valid(render->context) || !render->context->graphics_render_batch_quad_colored_uv) return VL_ERROR;
    return render->context->graphics_render_batch_quad_colored_uv(render, quad, brush, colors, uv);
}

vl_result_t vl_graphics_render_batch_quad(vl_graphics_render_t *render, vl_quad_t quad, vl_graphics_brush_t *brush) {
    return vl_graphics_render_batch_quad_colored(render, quad, brush, VL_QUAD_WHITE);
}

vl_result_t vl_graphics_render_batch_rect(vl_graphics_render_t *render, vl_rect_t rect, vl_graphics_brush_t *brush) {
    return vl_graphics_render_batch_rect_colored(render, rect, brush, VL_QUAD_WHITE);
}

vl_result_t vl_graphics_render_batch_rect_colored(vl_graphics_render_t *render, vl_rect_t rect, vl_graphics_brush_t *brush, vl_quad_colors_t colors) {
    return vl_graphics_render_batch_rect_colored_uv(render, rect, brush, colors, VL_QUAD_UV_DEFAULT);
}

vl_result_t vl_graphics_render_batch_rect_colored_uv(vl_graphics_render_t *render, vl_rect_t rect, vl_graphics_brush_t *brush, vl_quad_colors_t colors, vl_quad_uv_t uv) {
    return vl_graphics_render_batch_quad_colored_uv(render, VL_RECT_TO_QUAD(rect), brush, colors, uv);
}

vl_result_t vl_graphics_render_batch_point(vl_graphics_render_t *render, vl_point_t point, int size, vl_color_t color) {
    return vl_graphics_render_batch_rect_colored(render, VL_RECT(
        VL_POINT(point.x - (float) size / 2, point.y - (float) size / 2),
        VL_POINT(point.x + (float) size / 2, point.y + (float) size / 2)
    ), NULL, VL_QUAD_COLORS(color, color, color, color));
}

vl_result_t vl_graphics_render_batch_end(vl_graphics_render_t *render) {
    if (!render || !vl_platform_context_valid(render->context) || !render->context->graphics_render_batch_end) return VL_ERROR;
    return render->context->graphics_render_batch_end(render);
}

vl_result_t vl_graphics_render_resize(vl_graphics_render_t *render, int w, int h) {
    if (!render || !vl_platform_context_valid(render->context) || !render->context->graphics_render_resize) return VL_ERROR;
    return render->context->graphics_render_resize(render, w ,h);
}

vl_result_t vl_graphics_render_free(vl_graphics_render_t *render) {
    if (!render || !vl_platform_context_valid(render->context) || !render->context->graphics_render_free) return VL_ERROR;
    return render->context->graphics_render_free(render);
}