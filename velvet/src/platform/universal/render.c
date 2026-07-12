#include "platform/universal/render.h"
#include "graphics/render.h"
#include "platform/universal/window.h"
#include "support/da.h"
#include "support/memory.h"
#include "os/window.h"
#include "support/result.h"
#include "velvet.h"

#include <GLFW/glfw3.h>
#include <arm_neon.h>
#include <cglm/mat4.h>
#include <cglm/vec4.h>
#include <glad/gl.h>
#include <cglm/cam.h>

#define BATCH_MAX 256

static void ensure_context(GLFWwindow *window) {
    static GLFWwindow *s_current_context = NULL;
    if (window != s_current_context) {
        glfwMakeContextCurrent(window);
        s_current_context = window;
    }
}

static void ensure_render_context(vl_graphics_render_universal_t *r) {
    vl_os_window_universal_t *win = (vl_os_window_universal_t*) (r->base.owner);
    ensure_context(win->handle);
}

static const char *s_batch_vertex_shader = 
"#version 330 core\n"
VL_STRINGIFY(
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;

out vec4 vColor;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}
);

static const char *s_batch_fragment_shader = 
"#version 330 core\n"
VL_STRINGIFY(
out vec4 FragColor;

in vec4 vColor;

void main() {
    FragColor = vColor;
}
);

static inline void flat_ortho(float w, float h, mat4 dest) {
    glm_ortho(0, w, h, 0, -1.0f, 1.0f, dest);
}

GLuint compile_shader(GladGLContext *ctx, GLenum shader_type, const char *source) {
    static GLchar error_log[1024];
    GLuint shader = ctx->CreateShader(shader_type);
    ctx->ShaderSource(shader, 1, &source, NULL);
    ctx->CompileShader(shader);
    GLint success;
    ctx->GetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        ctx->GetShaderInfoLog(shader, sizeof(error_log), NULL, error_log);
        printf("%s\n", error_log);
    }
    return shader;
}

vl_graphics_render_t *vl_graphics_render_universal_new(vl_os_window_t *win) {
    if (!win) return NULL;
    vl_graphics_render_universal_t *render = VL_NEW(vl_graphics_render_universal_t);
    if (!render) {
        goto fail;
    }
    // assuming vl_os_window_t is using universal backend
    // TODO: implement backend cheking
    vl_os_window_universal_t *window = (vl_os_window_universal_t*) win;
    ensure_context(window->handle);
    if (!gladLoadGLContext(&render->ctx, glfwGetProcAddress)) {
        goto fail;
    }
    render->base.owner = win;
    render->base.backend = (const char*) render->ctx.GetString(GL_VERSION);
    if (!render->base.backend) {
        goto fail;
    }
    int fw, fh;
    int w, h;
    glfwGetFramebufferSize(window->handle, &fw, &fh);
    glfwGetWindowSize(window->handle, &w, &h);
    render->ctx.Viewport(0, 0, fw, fh);
    flat_ortho(w, h, render->proj_mat);
    printf("%i %i\n", w, h);

    render->ctx.GenBuffers(1, &render->batch_vbo);
    render->ctx.BindBuffer(GL_ARRAY_BUFFER, render->batch_vbo);
    render->ctx.BufferData(GL_ARRAY_BUFFER, BATCH_MAX * 6 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    
    render->batch_offset = 0;
    render->batch_vertices = VL_DA_INIT_WITH_CAPACITY(float, BATCH_MAX * 6);
    render->batch_active = false;

    render->ctx.GenVertexArrays(1, &render->batch_vao);
    render->ctx.BindVertexArray(render->batch_vao);
    render->ctx.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), NULL);
    render->ctx.EnableVertexAttribArray(0);
    render->ctx.VertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (2 * sizeof(float)));
    render->ctx.EnableVertexAttribArray(1);

    GLint vertex_shader, fragment_shader;
    vertex_shader = compile_shader(&render->ctx, GL_VERTEX_SHADER, s_batch_vertex_shader);
    fragment_shader = compile_shader(&render->ctx, GL_FRAGMENT_SHADER, s_batch_fragment_shader);

    render->batch_program = render->ctx.CreateProgram();
    render->ctx.AttachShader(render->batch_program, vertex_shader);
    render->ctx.AttachShader(render->batch_program, fragment_shader);
    render->ctx.LinkProgram(render->batch_program);
    GLint success;
    render->ctx.GetProgramiv(render->batch_program, GL_LINK_STATUS, &success);
    if (!success) {
        static GLchar error[1024];
        render->ctx.GetProgramInfoLog(render->batch_program, sizeof(error), NULL, error);
        printf("%s\n", error);
    }

    render->ctx.DeleteShader(vertex_shader);
    render->ctx.DeleteShader(fragment_shader);

    return (vl_graphics_render_t*) render;

    fail:
    vl_free(render);
    return NULL;
}

vl_result_t vl_graphics_render_universal_clear(vl_graphics_render_t *render, vl_color_t fill) {
    if (!render) return VL_ERROR;
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) render;
    ensure_render_context(r);
    r->ctx.ClearColor(fill.r, fill.g, fill.b, fill.a);
    r->ctx.Clear(GL_COLOR_BUFFER_BIT);
    return VL_SUCCESS;
}

vl_result_t vl_graphics_render_universal_batch_begin(vl_graphics_render_t *render) {
    if (!render) return VL_ERROR;
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) render;
    r->batch_offset = 0;
    r->batch_active = true;
    return VL_SUCCESS;
}

static void batch_add_vertex(vl_graphics_render_universal_t *render, float x, float y, vl_color_t color) {
    if (!render->batch_active) return;
    render->batch_vertices[render->batch_offset++] = x;
    render->batch_vertices[render->batch_offset++] = y;
    render->batch_vertices[render->batch_offset++] = color.r;
    render->batch_vertices[render->batch_offset++] = color.g;
    render->batch_vertices[render->batch_offset++] = color.b;
    render->batch_vertices[render->batch_offset++] = color.a;
}

vl_result_t vl_graphics_render_universal_batch_rect(vl_graphics_render_t *render, vl_rect_t rect, vl_color_t fill) {
    if (!render) return VL_ERROR;
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) render;
    batch_add_vertex(r, rect.x1, rect.y1, fill);
    batch_add_vertex(r, rect.x2, rect.y1, fill);
    batch_add_vertex(r, rect.x1, rect.y2, fill);
    batch_add_vertex(r, rect.x2, rect.y1, fill);
    batch_add_vertex(r, rect.x2, rect.y2, fill);
    batch_add_vertex(r, rect.x1, rect.y2, fill);
    return VL_SUCCESS;
}

vl_result_t vl_graphics_render_universal_batch_end(vl_graphics_render_t *render) {
    if (!render) return VL_ERROR;
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) render;
    r->batch_active = false;
    if (r->batch_offset == 0) return VL_SUCCESS;
    ensure_render_context(r);
    for (int i = 0; i < r->batch_offset; i += 6) {
        vec4 v = {r->batch_vertices[i], r->batch_vertices[i + 1], 0, 1};
        // printf("%0.2f %0.2f\n", r->batch_vertices[i], r->batch_vertices[i + 1]);
        glm_mat4_mulv(r->proj_mat, v, v);
        r->batch_vertices[i] = v[0];
        r->batch_vertices[i + 1] = v[1];
    }
    r->ctx.BindBuffer(GL_ARRAY_BUFFER, r->batch_vbo);
    r->ctx.BufferSubData(GL_ARRAY_BUFFER, 0, r->batch_offset * sizeof(float), r->batch_vertices);

    r->ctx.UseProgram(r->batch_program);
    r->ctx.BindVertexArray(r->batch_vao);
    r->ctx.DrawArrays(GL_TRIANGLES, 0, r->batch_offset / 6);
    return VL_SUCCESS;
}

vl_result_t vl_graphics_render_universal_free(vl_graphics_render_t *render) {
    if (!render) return VL_ERROR;
    vl_free(render);
    return VL_SUCCESS;
}