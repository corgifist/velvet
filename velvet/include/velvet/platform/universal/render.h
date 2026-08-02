/*
    platform/universal/render.h - implementation of vl_graphics_render using OpenGL 3.3
*/

#ifndef VELVET_PLATFORM_UNIVERSAL_RENDER_H
#define VELVET_PLATFORM_UNIVERSAL_RENDER_H

#include "graphics/bitmap.h"
#include "graphics/color.h"
#include "graphics/geometry.h"
#include "velvet/support/da.h"
#include "velvet/graphics/brush.h"
#include "velvet/os/window.h"
#include "velvet/graphics/render.h"
#include "velvet/platform/universal/bitmap.h"

#include <glad/gl.h>
#include <cglm/mat4.h>

struct Brush;
struct GradientStop;

typedef VL_PACK(struct {
    float x, y;
    int brush_index;
    union {
        vl_color_t color;
        struct {
            float r, g, b, a;
        };
    };
    float s, t;
}) vl_graphics_vertex_t;

struct vl_graphics_render_universal {
    vl_graphics_render_t base;

    GladGLContext ctx;
    GLuint batch_vbo, batch_vao;
    GLuint batch_program;
    VL_DA(vl_graphics_vertex_t) batch_vertices;
    int batch_offset;
    bool batch_active;

    GLuint brush_vbo;
    VL_DA(struct Brush) brush_da;
    int brush_offset;

    VL_DA(struct GradientStop) stops_da;
    int stops_offset;

    VL_DA(vl_graphics_brush_t*) owned_brushes;

    VL_DA(vl_graphics_brush_bitmap_t*) active_bitmaps;
    VL_DA(GLuint) active_samplers;
    size_t bitmap_offset;

    GLuint dummy_texture;

    mat4 proj_mat;
};

typedef struct vl_graphics_render_universal vl_graphics_render_universal_t;

vl_graphics_render_t *vl_graphics_render_universal_new(vl_os_window_t *window);
vl_result_t vl_graphics_render_universal_clear(vl_graphics_render_t *render, vl_color_t color);
vl_result_t vl_graphics_render_universal_batch_begin(vl_graphics_render_t *render);
vl_result_t vl_graphics_render_universal_batch_quad_colored_uv(vl_graphics_render_t *render, vl_quad_t quad, vl_graphics_brush_t *brush, vl_quad_colors_t colors, vl_quad_uv_t uv);
vl_result_t vl_graphics_render_universal_batch_end(vl_graphics_render_t *render);
vl_result_t vl_graphics_render_universal_resize(vl_graphics_render_t *render, int w, int h);
vl_result_t vl_graphics_render_universal_free(vl_graphics_render_t *render);

#endif // VELVET_PLATFORM_UNIVERSAL_GRAPHICS_H