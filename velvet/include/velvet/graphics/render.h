#ifndef VELVET_GRAPHICS_GRAPHICS_H
#define VELVET_GRAPHICS_GRAPHICS_H

#include "velvet/platform/context.h"
#include "velvet/graphics/brush.h"
#include "velvet/support/api.h"
#include "velvet/support/result.h"
#include "velvet/graphics/color.h"
#include "velvet/graphics/geometry.h"
#include "velvet/os/window.h"
#include <cglm/types.h>

struct vl_graphics_render_mat4 {
    float mat[16];
};

typedef struct vl_graphics_render_mat4 vl_graphics_render_mat4_t;

struct vl_graphics_render {
    vl_os_window_t *owner;
    const char *backend;

    vl_platform_context_t *context;

    VL_DA(vl_graphics_render_mat4_t) transform;
};
typedef struct vl_graphics_render vl_graphics_render_t;

VL_API vl_graphics_render_t *vl_graphics_render_new(vl_os_window_t *window);
VL_API vl_result_t vl_graphics_render_clear(vl_graphics_render_t *render, vl_color_t color);
VL_API vl_result_t vl_graphics_render_push_transform(vl_graphics_render_t *render, mat4 transform);
VL_API vl_result_t vl_graphics_render_push_translate(vl_graphics_render_t *render, vl_vec2_t translation);
VL_API vl_result_t vl_graphics_render_push_rotate(vl_graphics_render_t *render, float degrees);
VL_API vl_result_t vl_graphics_render_push_scale(vl_graphics_render_t *render, vl_vec2_t scale);
VL_API vl_result_t vl_graphics_render_batch_begin(vl_graphics_render_t *render);
VL_API vl_result_t vl_graphics_render_batch_vertex(vl_graphics_render_t *render, vl_vec2_t point, vl_graphics_brush_t *brush, vl_color_t color, vl_vec2_t uv);
VL_API vl_result_t vl_graphics_render_batch_quad(vl_graphics_render_t *render, vl_quad_t quad, vl_graphics_brush_t *brush);
VL_API vl_result_t vl_graphics_render_batch_quad_colored(vl_graphics_render_t *render, vl_quad_t quad, vl_graphics_brush_t *brush, vl_quad_colors_t colors);
VL_API vl_result_t vl_graphics_render_batch_quad_colored_uv(vl_graphics_render_t *render, vl_quad_t quad, vl_graphics_brush_t *brush, vl_quad_colors_t colors, vl_quad_uv_t uv);
VL_API vl_result_t vl_graphics_render_batch_rect(vl_graphics_render_t *render, vl_rect_t rect, vl_graphics_brush_t *brush);
VL_API vl_result_t vl_graphics_render_batch_rect_colored(vl_graphics_render_t *render, vl_rect_t rect, vl_graphics_brush_t *brush, vl_quad_colors_t colors);
VL_API vl_result_t vl_graphics_render_batch_rect_colored_uv(vl_graphics_render_t *render, vl_rect_t rect, vl_graphics_brush_t *brush, vl_quad_colors_t colors, vl_quad_uv_t uv);
VL_API vl_result_t vl_graphics_render_batch_point(vl_graphics_render_t *render, vl_point_t point, int size, vl_color_t color);
VL_API vl_result_t vl_graphics_render_batch_end(vl_graphics_render_t *render);
VL_API vl_result_t vl_graphics_render_pop_transform(vl_graphics_render_t *render);
VL_API vl_result_t vl_graphics_render_clear_transform(vl_graphics_render_t *render);
VL_API vl_result_t vl_graphics_render_resize(vl_graphics_render_t *render, int w, int h);
VL_API vl_result_t vl_graphics_render_free(vl_graphics_render_t *render);

#endif // VELVET_GRAPHICS_GRAPHICS_H