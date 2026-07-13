/*
    platform/universal/render.h - implementation of vl_graphics_render using OpenGL 3.3
*/

#ifndef VELVET_PLATFORM_UNIVERSAL_RENDER_H
#define VELVET_PLATFORM_UNIVERSAL_RENDER_H

#include "graphics/color.h"
#include "velvet/support/da.h"
#include "velvet/graphics/brush.h"
#include "velvet/os/window.h"
#include "velvet/graphics/render.h"

#include <glad/gl.h>
#include <cglm/mat4.h>

struct Brush;

struct vl_graphics_render_universal {
    vl_graphics_render_t base;

    GladGLContext ctx;
    GLuint batch_vbo, batch_vao;
    GLuint batch_program;
    VL_DA(float) batch_vertices;
    int batch_offset;
    bool batch_active;

    GLuint brush_vbo;
    VL_DA(struct Brush) brush_da;
    int brush_offset;

    VL_DA(vl_graphics_brush_t*) owned_brushes;

    mat4 proj_mat;
};

typedef struct vl_graphics_render_universal vl_graphics_render_universal_t;

vl_graphics_render_t *vl_graphics_render_universal_new(vl_os_window_t *window);
vl_result_t vl_graphics_render_universal_clear(vl_graphics_render_t *render, vl_color_t fill);
vl_result_t vl_graphics_render_universal_batch_begin(vl_graphics_render_t *render);
vl_result_t vl_graphics_render_universal_batch_quad_colored(vl_graphics_render_t *render, vl_quad_t quad, vl_graphics_brush_t *brush, vl_quad_colors_t colors);
vl_result_t vl_graphics_render_universal_batch_rect_colored(vl_graphics_render_t *render, vl_rect_t rect, vl_graphics_brush_t *brush, vl_quad_colors_t colors);
vl_result_t vl_graphics_render_universal_batch_end(vl_graphics_render_t *render);
vl_result_t vl_graphics_render_universal_free(vl_graphics_render_t *render);

#endif // VELVET_PLATFORM_UNIVERSAL_GRAPHICS_H