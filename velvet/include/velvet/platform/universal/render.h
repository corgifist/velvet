/*
    platform/universal/render.h - implementation of vl_graphics_render using OpenGL 3.3
*/

#ifndef VELVET_PLATFORM_UNIVERSAL_RENDER_H
#define VELVET_PLATFORM_UNIVERSAL_RENDER_H

#include "velvet/os/window.h"
#include "velvet/graphics/render.h"

#include <glad/gl.h>
#include <cglm/mat4.h>

struct vl_graphics_render_universal {
    vl_graphics_render_t base;

    GladGLContext ctx;
    GLuint quad_vbo, quad_vao;
    GLuint quad_program;

    mat4 proj_mat;
};

typedef struct vl_graphics_render_universal vl_graphics_render_universal_t;

vl_graphics_render_t *vl_graphics_render_universal_new(vl_os_window_t *window);
vl_result_t vl_graphics_render_universal_clear(vl_graphics_render_t *render, vl_color_t fill);
vl_result_t vl_graphics_render_universal_rect(vl_graphics_render_t *render, vl_rect_t rect, vl_color_t fill);
vl_result_t vl_graphics_render_universal_free(vl_graphics_render_t *render);

#endif // VELVET_PLATFORM_UNIVERSAL_GRAPHICS_H