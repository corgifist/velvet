#ifndef VELVET_PLATFORM_CONTEXT_H
#define VELVET_PLATFORM_CONTEXT_H

#include "velvet/common.h"
#include "velvet/support/result.h"
#include "velvet/support/api.h"
#include "velvet/support/memory.h"
#include "velvet/graphics/geometry.h"
#include "velvet/graphics/color.h"
#include "velvet/graphics/bitmap.h"

enum vl_platform_context_type {
    VL_PLATFORM_CONTEXT_UNIVERSAL = 0
};

typedef enum vl_platform_context_type vl_platform_context_type_t;

struct vl_platform_context_types {
    vl_platform_context_type_t os_window;
    vl_platform_context_type_t os_sleep;
    vl_platform_context_type_t graphics_render;
    vl_platform_context_type_t graphics_presentation;
    vl_platform_context_type_t font;
};

typedef struct vl_platform_context_types vl_platform_context_types_t;

#define VL_PLATFORM_CONTEXT_TYPES() \
    ((vl_platform_context_types_t) {0})

#define VL_PLATFORM_CONTEXT_DEFAULT \
    VL_PLATFORM_CONTEXT_TYPES()

struct vl_platform_context;
// velvet/os/window.h
struct vl_os_window;
typedef struct vl_os_window* (*vl_ctx_os_window_new)(const char *title, int w, int h);
typedef vl_result_t (*vl_ctx_os_window_should_close)(struct vl_os_window *window, bool *should_close);
typedef vl_result_t (*vl_ctx_os_window_poll_events)();
typedef vl_result_t (*vl_ctx_os_window_free)(struct vl_os_window *window);

// velvet/os/sleep.h
typedef void (*vl_ctx_os_sleep)(uint32_t nanoseconds);

// velvet/graphics/render.h
struct vl_graphics_render;
struct vl_graphics_brush;
typedef struct vl_graphics_render* (*vl_ctx_graphics_render_new)(struct vl_os_window *window);
typedef vl_result_t (*vl_ctx_graphics_render_clear)(struct vl_graphics_render *render, vl_color_t fill);
typedef vl_result_t (*vl_ctx_graphics_render_batch_begin)(struct vl_graphics_render *render);
typedef vl_result_t (*vl_ctx_graphics_render_batch_quad_colored_uv)(struct vl_graphics_render *render, vl_quad_t quad, struct vl_graphics_brush *brush, vl_quad_colors_t colors, vl_quad_uv_t uv);
typedef vl_result_t (*vl_ctx_graphics_render_batch_end)(struct vl_graphics_render *render);
typedef vl_result_t (*vl_ctx_graphics_render_resize)(struct vl_graphics_render *render, int w, int h);
typedef vl_result_t (*vl_ctx_graphics_render_free)(struct vl_graphics_render *render);

// velvet/graphics/presentation.h
struct vl_graphics_presentation;
typedef struct vl_graphics_presentation* (*vl_ctx_graphics_presentation_new)(struct vl_os_window *window, struct vl_graphics_render *render);
typedef vl_result_t (*vl_ctx_graphics_presentation_begin)(struct vl_graphics_presentation *presentation);
typedef vl_result_t (*vl_ctx_graphics_presentation_end)(struct vl_graphics_presentation *presentation);
typedef vl_result_t (*vl_ctx_graphics_presentation_free)(struct vl_graphics_presentation *presentation);

// velvet/graphics/bitmap.h
struct vl_graphics_bitmap;
typedef struct vl_graphics_bitmap* (*vl_ctx_graphics_bitmap_new)(struct vl_graphics_render *render, 
    size_t width, size_t height, enum vl_graphics_bitmap_format format, void *data);
typedef vl_result_t (*vl_ctx_graphics_bitmap_free)(struct vl_graphics_bitmap *bitmap);

// velvet/graphics/brush.h
struct vl_graphics_brush;
typedef struct vl_graphics_brush* (*vl_ctx_graphics_brush_new_solid)(struct vl_graphics_render *render, vl_color_t color);
typedef struct vl_graphics_brush* (*vl_ctx_graphics_brush_new_linear_gradient)(struct vl_graphics_render *render, vl_gradient_stop_t *stops, size_t stops_count);
typedef struct vl_graphics_brush *(*vl_ctx_graphics_brush_new_bitmap)(struct vl_graphics_render *render, struct vl_graphics_bitmap *bitmap);
typedef vl_result_t (*vl_ctx_graphics_brush_free)(struct vl_graphics_brush *brush);

// velvet/font/font.h
struct vl_font;
struct vl_font_atlas_codepoint;
struct vl_font_atlas;
typedef struct vl_font* (*vl_ctx_font_new)(struct vl_platform_context *context, const char *name, int height, float density, const vl_byte_t *data, size_t data_length, vl_source_location_t loc);
typedef struct vl_font_atlas_codepoint* (*vl_ctx_font_rasterize_codepoint)(struct vl_font *font, struct vl_font_atlas *atlas, uint32_t codepoint);
typedef float (*vl_ctx_font_get_kern_advance)(struct vl_font *font, uint32_t codepoint_a, uint32_t codepoint_b);
typedef vl_result_t (*vl_ctx_font_free)(struct vl_font *font);

// velvet/font/shaper.h
struct vl_font_shaper;
struct vl_font_shaper_run;
struct vl_font_shaper_glyph;
typedef struct vl_font_shaper_run* (*vl_ctx_font_shaper_run_new)(struct vl_font_shaper *shaper, vl_source_location_t loc);
typedef struct vl_font_shaper *(*vl_ctx_font_shaper_new)(struct vl_platform_context *context, vl_source_location_t loc);
typedef vl_result_t (*vl_ctx_font_shaper_add_font)(struct vl_font_shaper *shaper, struct vl_font *font);
typedef vl_result_t (*vl_ctx_font_shaper_process)(struct vl_font_shaper *shaper, const char *text, size_t text_length);
typedef bool (*vl_ctx_font_shaper_shape)(struct vl_font_shaper *shaper, struct vl_font_shaper_run *run);
typedef struct vl_font_shaper_glyph* (*vl_ctx_font_shaper_iterate)(struct vl_font_shaper_run *run, struct vl_font_shaper_glyph *glyph);
typedef vl_result_t (*vl_ctx_font_shaper_run_reset)(struct vl_font_shaper_run *run);
typedef vl_result_t (*vl_ctx_font_shaper_run_free)(struct vl_font_shaper_run *run);
typedef vl_result_t (*vl_ctx_font_shaper_free)(struct vl_font_shaper *shaper);

struct vl_platform_context {
    vl_platform_context_types_t types;

    vl_ctx_os_window_new os_window_new;
    vl_ctx_os_window_should_close os_window_should_close;
    vl_ctx_os_window_poll_events os_window_poll_events;
    vl_ctx_os_window_free os_window_free;

    vl_ctx_os_sleep os_sleep;

    vl_ctx_graphics_render_new graphics_render_new;
    vl_ctx_graphics_render_clear graphics_render_clear;
    vl_ctx_graphics_render_batch_begin graphics_render_batch_begin;
    vl_ctx_graphics_render_batch_quad_colored_uv graphics_render_batch_quad_colored_uv;
    vl_ctx_graphics_render_batch_end graphics_render_batch_end;
    vl_ctx_graphics_render_resize graphics_render_resize;
    vl_ctx_graphics_render_free graphics_render_free;

    vl_ctx_graphics_presentation_new graphics_presentation_new;
    vl_ctx_graphics_presentation_begin graphics_presentation_begin;
    vl_ctx_graphics_presentation_end graphics_presentation_end;
    vl_ctx_graphics_presentation_free graphics_presentation_free;

    vl_ctx_graphics_bitmap_new graphics_bitmap_new;
    vl_ctx_graphics_bitmap_free graphics_bitmap_free;

    vl_ctx_graphics_brush_new_solid graphics_brush_new_solid;
    vl_ctx_graphics_brush_new_linear_gradient graphics_brush_new_linear_gradient;
    vl_ctx_graphics_brush_new_bitmap graphics_brush_new_bitmap;
    vl_ctx_graphics_brush_free graphics_brush_free;

    vl_ctx_font_new font_new;
    vl_ctx_font_rasterize_codepoint font_rasterize_codepoint;
    vl_ctx_font_get_kern_advance font_get_kern_advance;
    vl_ctx_font_free font_free;

    vl_ctx_font_shaper_new font_shaper_new;
    vl_ctx_font_shaper_add_font font_shaper_add_font;
    vl_ctx_font_shaper_process font_shaper_process;
    vl_ctx_font_shaper_run_new font_shaper_run_new;
    vl_ctx_font_shaper_shape font_shaper_shape;
    vl_ctx_font_shaper_iterate font_shaper_iterate;
    vl_ctx_font_shaper_run_reset font_shaper_run_reset;
    vl_ctx_font_shaper_run_free font_shaper_run_free;
    vl_ctx_font_shaper_free font_shaper_free;
};

typedef struct vl_platform_context vl_platform_context_t;

#define vl_platform_context_new(types) \
    vl_platform_context_new_(types, VL_SOURCE_LOCATION_HERE)
VL_API vl_platform_context_t *vl_platform_context_new_(vl_platform_context_types_t types, vl_source_location_t loc);
VL_API vl_result_t vl_platform_context_free(vl_platform_context_t *context);
VL_API bool vl_platform_context_valid(vl_platform_context_t *context);

#endif // VELVET_PLATFORM_CONTEXT_H