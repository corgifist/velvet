#include "velvet/graphics/render.h"
#include "graphics/brush.h"
#include "support/base_math.h"
#include "support/color.h"
#include "platform/context.h"
#include "support/da.h"
#include "support/math.h"
#include "support/result.h"

vl_graphics_render_t *vl_graphics_render_new(vl_os_window_t *window) {
    if (!window || !vl_platform_context_valid(window->context) || !window->context->graphics_render_new) return NULL;
    vl_graphics_render_t *render = window->context->graphics_render_new(window);
    if (render) {
        render->context = window->context;
        render->transform = VL_DA_INIT(vl_mat4_t);
    }
    return render;
}

vl_result_t vl_graphics_render_clear(vl_graphics_render_t *render, vl_color_t color) {
    if (!render || !vl_platform_context_valid(render->context) || !render->context->graphics_render_clear) return VL_ERROR;
    return render->context->graphics_render_clear(render, color);
}

vl_result_t vl_graphics_render_push_transform(vl_graphics_render_t *render, vl_mat4_t transform) {
    if (!render) return VL_ERROR;
    if (render->transform) {
        vl_mat4_t *mat = VL_DA_PUSH(render->transform, vl_mat4_t);
        vl_mat4_dup(mat, transform);
    }
    return VL_SUCCESS;
}

vl_result_t vl_graphics_render_push_translate(vl_graphics_render_t *render, vl_vec2_t translation) {
    if (!render) return VL_ERROR;
    vl_mat4_t mat = VL_MAT4();
    vl_mat4_translate(&mat, mat, translation.x, translation.y, 0);
    vl_graphics_render_push_transform(render, mat);
    return VL_SUCCESS;
}

vl_result_t vl_graphics_render_push_rotate(vl_graphics_render_t *render, float degrees) {
    if (!render) return VL_ERROR;
    vl_mat4_t mat = VL_MAT4();
    vl_mat4_rotate(&mat, mat, VL_VEC3(0, 0, 1), VL_DEG2RAD * degrees);
    vl_graphics_render_push_transform(render, mat);
    return VL_SUCCESS;
}

vl_result_t vl_graphics_render_push_scale(vl_graphics_render_t *render, vl_vec2_t scale) {
    if (!render) return VL_ERROR;
    vl_mat4_t mat = VL_MAT4();
    vl_mat4_scale_aniso(&mat, mat, VL_VEC3(scale.x, scale.y, 1));
    vl_graphics_render_push_transform(render, mat);
    return VL_SUCCESS;
}

vl_result_t vl_graphics_render_batch_begin(vl_graphics_render_t *render) {
    if (!render || !vl_platform_context_valid(render->context) || !render->context->graphics_render_batch_begin) return VL_ERROR;
    return render->context->graphics_render_batch_begin(render);
}

vl_result_t vl_graphics_render_batch_vertex(vl_graphics_render_t *render, vl_vec2_t point, vl_graphics_brush_t *brush, vl_color_t color, vl_vec2_t uv) {
    if (!render || !vl_platform_context_valid(render->context) || !render->context->graphics_render_batch_vertex) return VL_ERROR;
    if (render->transform) {
        for (int i = 0; i < VL_DA_LENGTH(render->transform); i++) {
            vl_vec4_t src = {point.x, point.y, 0, 1};
            vl_vec4_t dst;
            vl_mat4_mul_vec4(&dst, render->transform[i], src);
            point = VL_POINT(dst.x, dst.y);
        }
    }
    return render->context->graphics_render_batch_vertex(render, point, brush, color, uv); 
}

vl_result_t vl_graphics_render_batch_quad_colored(vl_graphics_render_t *render, vl_quad_t quad, vl_graphics_brush_t *brush, vl_quad_colors_t colors) {
    return vl_graphics_render_batch_quad_colored_uv(render, quad, brush, colors, VL_QUAD_UV_DEFAULT);
}

vl_result_t vl_graphics_render_batch_quad_colored_uv(vl_graphics_render_t *render, vl_quad_t quad, vl_graphics_brush_t *brush, vl_quad_colors_t colors, vl_quad_uv_t uv) {
    if (!render || !vl_platform_context_valid(render->context)) return VL_ERROR;
    if (vl_graphics_render_batch_vertex(render, quad.p2, brush, colors.tr, uv.tr)) return VL_ERROR;
    if (vl_graphics_render_batch_vertex(render, quad.p4, brush, colors.bl, uv.bl)) return VL_ERROR;
    if (vl_graphics_render_batch_vertex(render, quad.p3, brush, colors.br, uv.br)) return VL_ERROR;
    if (vl_graphics_render_batch_vertex(render, quad.p2, brush, colors.tr, uv.tr)) return VL_ERROR;
    if (vl_graphics_render_batch_vertex(render, quad.p1, brush, colors.tl, uv.tl)) return VL_ERROR;
    if (vl_graphics_render_batch_vertex(render, quad.p4, brush, colors.bl, uv.bl)) return VL_ERROR;
    return VL_SUCCESS;
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

vl_result_t vl_graphics_render_pop_transform(vl_graphics_render_t *render) {
    if (!render) return VL_ERROR;
    if (render->transform) {
        if (VL_DA_LENGTH(render->transform) <= 0) return VL_ERROR;
        VL_DA_DELETE(render->transform, VL_DA_LENGTH(render->transform) - 1);
    }
    return VL_SUCCESS;
}

vl_result_t vl_graphics_render_clear_transform(vl_graphics_render_t *render) {
    if (!render) return VL_ERROR;
    VL_DA_FREE(render->transform);
    render->transform = VL_DA_INIT(vl_mat4_t);
    return VL_SUCCESS;
}

vl_result_t vl_graphics_render_resize(vl_graphics_render_t *render, int w, int h) {
    if (!render || !vl_platform_context_valid(render->context) || !render->context->graphics_render_resize) return VL_ERROR;
    return render->context->graphics_render_resize(render, w ,h);
}

vl_result_t vl_graphics_render_free(vl_graphics_render_t *render) {
    if (!render || !vl_platform_context_valid(render->context) || !render->context->graphics_render_free) return VL_ERROR;
    VL_DA_FREE(render->transform);
    return render->context->graphics_render_free(render);
}