#include "platform/universal/render.h"
#include "graphics/bitmap.h"
#include "graphics/brush.h"
#include "graphics/color.h"
#include "graphics/render.h"
#include "platform/universal/bitmap.h"
#include "platform/universal/window.h"
#include "platform/universal/brush.h"
#include "support/da.h"
#include "support/memory.h"
#include "os/window.h"
#include "support/result.h"
#include "support/str.h"
#include "velvet.h"

#include <GLFW/glfw3.h>
#include <cglm/mat4.h>
#include <cglm/types.h>
#include <cglm/vec4.h>
#include <glad/gl.h>
#include <cglm/cam.h>
#include <string.h>

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
layout(location = 1) in int aBrushIndex;
layout(location = 2) in vec4 aColor;
layout(location = 3) in vec2 aST;

flat out int vBrushIndex;
out vec4 vColor;
out vec2 vST;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vBrushIndex = aBrushIndex;
    vColor = aColor;
    vST = aST;
}
);

#define BRUSH_MAX 16
#define STOPS_MAX 32
#define BITMAP_MAX 16

typedef VL_PACK(struct Brush {
    ivec4 brush_data;
    vec4 color;
}) Brush;

typedef VL_PACK(struct GradientStop {
    float percentage;
    vec3 pad;
    vec4 color;
}) GradientStop;

static const char *s_batch_fragment_shader = 
"#version 330 core\n"
"#define BRUSH_SOLID 1\n"
"#define BRUSH_LINEAR_GRADIENT 2\n"
"#define BRUSH_BITMAP 3\n"
VL_STRINGIFY(
out vec4 FragColor;

flat in int vBrushIndex;
in vec4 vColor;
in vec2 vST;

struct Brush {
    ivec4 brush_data;
    vec4 color;
};

struct GradientStop {
    float percentage;
    vec4 color;
};

layout(std140) uniform BrushData {
    Brush brushes[BRUSH_MAX];
    GradientStop stops[STOPS_MAX];
};

uniform sampler2D bitmaps[BITMAP_MAX];

vec4 sampleBitmap(int bitmapIndex, vec2 st) {
    if (bitmapIndex == 0) { return texture(bitmaps[0], st); }
    if (bitmapIndex == 1) { return texture(bitmaps[1], st); }
    if (bitmapIndex == 2) { return texture(bitmaps[2], st); }
    if (bitmapIndex == 3) { return texture(bitmaps[3], st); }
    if (bitmapIndex == 4) { return texture(bitmaps[4], st); }
    if (bitmapIndex == 5) { return texture(bitmaps[5], st); }
    if (bitmapIndex == 6) { return texture(bitmaps[6], st); }
    if (bitmapIndex == 7) { return texture(bitmaps[7], st); }
    if (bitmapIndex == 8) { return texture(bitmaps[8], st); }
    if (bitmapIndex == 9) { return texture(bitmaps[9], st); }
    if (bitmapIndex == 10) { return texture(bitmaps[10], st); }
    if (bitmapIndex == 11) { return texture(bitmaps[11], st); }
    if (bitmapIndex == 12) { return texture(bitmaps[12], st); }
    if (bitmapIndex == 13) { return texture(bitmaps[13], st); }
    if (bitmapIndex == 14) { return texture(bitmaps[14], st); }
    if (bitmapIndex == 15) { return texture(bitmaps[15], st); }
}

void main() {
    vec4 color = vColor;
    if (vBrushIndex >= 0) {
        if (brushes[vBrushIndex].brush_data.x == BRUSH_SOLID) {
            color *= brushes[vBrushIndex].color;
        } else if (brushes[vBrushIndex].brush_data.x == BRUSH_LINEAR_GRADIENT) {
            int first_stop = brushes[vBrushIndex].brush_data.y;
            int stops_count = brushes[vBrushIndex].brush_data.z;
            int direction_mask = brushes[vBrushIndex].brush_data.w;
            vec2 begin = brushes[vBrushIndex].color.xy;
            vec2 end = brushes[vBrushIndex].color.zw;
            vec2 diff = end - begin;
            float denom = dot(diff, diff);
            float phase = (denom > 0.0) ? dot(vST - begin, diff) : 0.0;
            phase = clamp(phase, 0.0, 1.0);
            vec4 gradient_color = stops[first_stop].color;
            if (phase < stops[first_stop].percentage) {
                gradient_color = stops[first_stop].color;
            } else if (phase > stops[first_stop + stops_count - 1].percentage) {
                gradient_color = stops[first_stop + stops_count - 1].color;
            } else {
                for (int i = 0; i < stops_count - 1; i++) {
                    if (phase > stops[i].percentage && phase < stops[i + 1].percentage) {
                        gradient_color = mix(stops[i].color, stops[i + 1].color, (phase - stops[i].percentage) / (stops[i + 1].percentage - stops[i].percentage));
                        break;
                    }
                }
            }
            color *= gradient_color;
        } else if (brushes[vBrushIndex].brush_data.x == BRUSH_BITMAP) {
            color *= sampleBitmap(brushes[vBrushIndex].brush_data.y, vST);
        }
    }
    FragColor = color;
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
    // printf("%i %i\n", w, h);

    render->ctx.GenBuffers(1, &render->batch_vbo);
    render->ctx.BindBuffer(GL_ARRAY_BUFFER, render->batch_vbo);
    render->ctx.BufferData(GL_ARRAY_BUFFER, BATCH_MAX * (6 * sizeof(float) + sizeof(int)), NULL, GL_DYNAMIC_DRAW);

    render->batch_offset = 0;
    render->batch_vertices = VL_DA_INIT_WITH_CAPACITY(vl_graphics_vertex_t, BATCH_MAX);
    render->batch_active = false;

    render->ctx.GenBuffers(1, &render->brush_vbo);
    render->ctx.BindBuffer(GL_UNIFORM_BUFFER, render->brush_vbo);
    render->ctx.BufferData(GL_UNIFORM_BUFFER, BRUSH_MAX * sizeof(Brush) + STOPS_MAX * sizeof(GradientStop), NULL, GL_DYNAMIC_READ);

    render->brush_offset = 0;
    render->brush_da = VL_DA_INIT_WITH_CAPACITY(Brush, BRUSH_MAX);
    render->owned_brushes = VL_DA_INIT(vl_graphics_brush_t*);

    render->stops_da = VL_DA_INIT_WITH_CAPACITY(GradientStop, STOPS_MAX);
    render->stops_offset = 0;

    render->ctx.GenVertexArrays(1, &render->batch_vao);
    render->ctx.BindVertexArray(render->batch_vao);
    render->ctx.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float) + sizeof(int), NULL);
    render->ctx.EnableVertexAttribArray(0);
    render->ctx.VertexAttribIPointer(1, 1, GL_INT, 8 * sizeof(float) + sizeof(int), (void*) (2 * sizeof(float)));
    render->ctx.EnableVertexAttribArray(1);
    render->ctx.VertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float) + sizeof(int), (void*) (2 * sizeof(float) + sizeof(int)));
    render->ctx.EnableVertexAttribArray(2);
    render->ctx.VertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float) + sizeof(int), (void*) (6 * sizeof(float) + sizeof(int)));
    render->ctx.EnableVertexAttribArray(3);

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

    render->ctx.UseProgram(render->batch_program);
    for (int i = 0; i < BITMAP_MAX; i++) {
        int sampler_location = render->ctx.GetUniformLocation(render->batch_program, vl_sprintf_tmp("bitmaps[%i]", i));
        render->ctx.Uniform1i(sampler_location, i);
    }

    render->active_samplers = VL_DA_INIT_WITH_CAPACITY(GLint, BITMAP_MAX);
    render->ctx.GenSamplers(BITMAP_MAX, render->active_samplers);

    render->active_bitmaps = VL_DA_INIT_WITH_CAPACITY(vl_graphics_brush_bitmap_t*, BITMAP_MAX);
    render->bitmap_offset = 0;
    
    GLuint brushes_ubo = render->ctx.GetUniformBlockIndex(render->batch_program, "BrushData");
    render->ctx.UniformBlockBinding(render->batch_program, brushes_ubo, 0);

    GLint max_texture_size, max_texture_units;
    render->ctx.GetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
    render->ctx.GetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);
    printf("max texture size: %i\n", max_texture_size);
    printf("max texture units: %i\n", max_texture_units);

    VL_DA_APPEND(win->owned_renders, render);
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
    r->brush_offset = 0;
    r->stops_offset = 0;
    r->bitmap_offset = 0;
    return VL_SUCCESS;
}

static void batch_add_vertex(vl_graphics_render_t *render, float x, float y, int brush_index, vl_color_t color, float s, float t) {
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) render;
    if (!r) return;
    if (!r->batch_active) return;
    r->batch_vertices[r->batch_offset++] = (vl_graphics_vertex_t) {
        x, y, brush_index, color, s, t
    };
}

static int add_brush(vl_graphics_render_universal_t *render, Brush brush) {
    if (!render->batch_active) return -1;
    render->brush_da[render->brush_offset++] = brush;
    return render->brush_offset - 1;
}

static int get_brush_index(vl_graphics_render_t *render, vl_graphics_brush_t *brush) {
    if (!brush) return -1;
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) render;
    vl_brush_index_t *saved_brush_index = VL_PTR_BACKWARD(brush, sizeof(vl_brush_index_t));
    int brush_index;
    if (*saved_brush_index >= 0) {
        brush_index = *saved_brush_index;
    } else {
        switch (brush->type) {
        case VL_GRAPHICS_RENDER_BRUSH_SOLID: {
            vl_color_t brush_color = ((vl_graphics_brush_solid_t*) brush)->color;
            brush_index = add_brush(r, (Brush) {
                {1, 0, 0, 0}, {brush_color.r, brush_color.g, brush_color.b, brush_color.a}
            });
            break;
        }
        case VL_GRAPHICS_RENDER_BRUSH_LINEAR_GRADIENT: {
            vl_graphics_brush_linear_gradient_t *l = (vl_graphics_brush_linear_gradient_t*) brush;
            int gradient_begin_index = r->stops_offset;
            for (int i = 0; i < VL_DA_LENGTH(l->stops); i++) {
                *VL_DA_PUSH(r->stops_da, GradientStop) = (GradientStop) {
                    l->stops[i].percentage, {0}, {
                        l->stops[i].color.r, l->stops[i].color.g, 
                        l->stops[i].color.b, l->stops[i].color.a
                    }
                };
                r->stops_offset++;
            }
            brush_index = add_brush(r, (Brush) {
                {2, gradient_begin_index, VL_DA_LENGTH(l->stops), 0},
                {l->start.x, l->start.y, l->end.x, l->end.y}
            });
            break;
        }
        case VL_GRAPHICS_RENDER_BRUSH_BITMAP: {
            vl_graphics_brush_bitmap_t *b = (vl_graphics_brush_bitmap_t*) brush;
            vl_graphics_bitmap_universal_t *bu = (vl_graphics_bitmap_universal_t*) b->bitmap;
            int bitmap_index = -1;
            for (int i = 0; i < r->bitmap_offset; i++) {
                if (((vl_graphics_bitmap_universal_t*) r->active_bitmaps[i]->bitmap)->handle == bu->handle) {
                    bitmap_index = i;
                    break;
                }
            }
            if (bitmap_index < 0) {
                r->active_bitmaps[(bitmap_index = r->bitmap_offset++)] = b;
            }
            brush_index = add_brush(r, (Brush) {
                {3, bitmap_index, 0, 0},
                {0, 0, 0 ,0}
            });
            break;
        }
        }
    }
    if (brush_index >= 0) {
        *saved_brush_index = brush_index;
    }
    return brush_index;
}

vl_result_t vl_graphics_render_universal_batch_quad_colored(vl_graphics_render_t *render, vl_quad_t quad, vl_graphics_brush_t *brush, vl_quad_colors_t colors) {
    if (!render) return VL_ERROR;
    int brush_index = get_brush_index(render, brush);
    batch_add_vertex(render, quad.x2, quad.y2, brush_index, colors.tr, 1, 0);
    batch_add_vertex(render, quad.x4, quad.y4, brush_index, colors.bl, 0, 1);
    batch_add_vertex(render, quad.x3, quad.y3, brush_index, colors.br, 1, 1);
    batch_add_vertex(render, quad.x2, quad.y2, brush_index, colors.tr, 1, 0);
    batch_add_vertex(render, quad.x1, quad.y1, brush_index, colors.tl, 0, 0);
    batch_add_vertex(render, quad.x4, quad.y4, brush_index, colors.bl, 0, 1);
    return VL_SUCCESS;
}

static GLint interpret_extend_mode(vl_graphics_brush_extend_mode_t extend_mode) {
    switch (extend_mode) {
        case VL_GRAPHICS_BRUSH_EXTEND_CLAMP: return GL_CLAMP_TO_EDGE;
        case VL_GRAPHICS_BRUSH_EXTEND_MIRROR: return GL_MIRRORED_REPEAT;
        case VL_GRAPHICS_BRUSH_EXTEND_WRAP: return GL_REPEAT;
        default: return GL_REPEAT;
    }
}

static GLint interpret_filter_mode(vl_graphics_brush_bitmap_filter_t filter) {
    switch (filter) {
        case VL_GRAPHICS_BRUSH_BITMAP_FILTER_LINEAR: return GL_LINEAR;
        case VL_GRAPHICS_BRUSH_BITMAP_FILTER_NEAREST: return GL_NEAREST;
        default: return GL_LINEAR;
    }
}

vl_result_t vl_graphics_render_universal_batch_end(vl_graphics_render_t *render) {
    if (!render) return VL_ERROR;
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) render;
    r->batch_active = false;
    if (r->batch_offset == 0) return VL_SUCCESS;
    ensure_render_context(r);
    for (int i = 0; i < r->batch_offset; i++) {
        vec4 v = {r->batch_vertices[i].x, r->batch_vertices[i].y, 0, 1};
        glm_mat4_mulv(r->proj_mat, v, v);
        r->batch_vertices[i].x = v[0];
        r->batch_vertices[i].y = v[1];
    }
    r->ctx.BindBuffer(GL_ARRAY_BUFFER, r->batch_vbo);
    r->ctx.BufferSubData(GL_ARRAY_BUFFER, 0, r->batch_offset * sizeof(vl_graphics_vertex_t), r->batch_vertices);

    r->ctx.BindBuffer(GL_UNIFORM_BUFFER, r->brush_vbo);
    r->ctx.BufferSubData(GL_UNIFORM_BUFFER, 0, r->brush_offset * sizeof(Brush), r->brush_da);
    r->ctx.BufferSubData(GL_UNIFORM_BUFFER, BRUSH_MAX * sizeof(Brush), r->stops_offset * sizeof(GradientStop), r->stops_da);

    r->ctx.UseProgram(r->batch_program);
    for (int i = 0; i < r->bitmap_offset; i++) {
        vl_graphics_brush_bitmap_t *bitmap_brush = r->active_bitmaps[i];
        r->ctx.SamplerParameteri(r->active_samplers[i], GL_TEXTURE_WRAP_S, interpret_extend_mode(bitmap_brush->base.extend_x));
        r->ctx.SamplerParameteri(r->active_samplers[i], GL_TEXTURE_WRAP_T, interpret_extend_mode(bitmap_brush->base.extend_y));
        r->ctx.SamplerParameteri(r->active_samplers[i], GL_TEXTURE_MAG_FILTER, interpret_filter_mode(bitmap_brush->filter));
        r->ctx.SamplerParameteri(r->active_samplers[i], GL_TEXTURE_MIN_FILTER, interpret_filter_mode(bitmap_brush->filter));

        vl_graphics_bitmap_universal_t *bitmap = (vl_graphics_bitmap_universal_t*) bitmap_brush->bitmap;
        r->ctx.BindSampler(i, r->active_samplers[i]);
        r->ctx.ActiveTexture(GL_TEXTURE0 + i);
        r->ctx.BindTexture(GL_TEXTURE_2D, bitmap->handle);
    }
    r->ctx.BindVertexArray(r->batch_vao);
    r->ctx.BindBufferBase(GL_UNIFORM_BUFFER, 0, r->brush_vbo);
    r->ctx.DrawArrays(GL_TRIANGLES, 0, r->batch_offset);

    for (int i = 0; i < VL_DA_LENGTH(r->owned_brushes); i++) {
        // reset brush's saved brush index
        *((int*) (((vl_byte_t*) r->owned_brushes[i]) - sizeof(int))) = -1;
    }
    return VL_SUCCESS;
}

vl_result_t vl_graphics_render_universal_resize(vl_graphics_render_t *render, int w, int h) {
    if (!render) return VL_ERROR;
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) render;
    vl_os_window_universal_t *win = (vl_os_window_universal_t*) render->owner;
    flat_ortho(w, h, r->proj_mat);

    int fw, fh;
    glfwGetFramebufferSize(win->handle, &fw, &fh);
    r->ctx.Viewport(0, 0, fw, fh);
    return VL_SUCCESS;
}

vl_result_t vl_graphics_render_universal_free(vl_graphics_render_t *render) {
    if (!render) return VL_ERROR;
    vl_graphics_render_universal_t *u = (vl_graphics_render_universal_t*) render;
    VL_DA_FREE(u->batch_vertices);
    VL_DA_FREE(u->brush_da);
    VL_DA_FREE(u->stops_da);
    VL_DA_FREE(u->owned_brushes);
    vl_free(render);
    return VL_SUCCESS;
}