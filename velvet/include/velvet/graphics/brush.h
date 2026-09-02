#ifndef VELVET_GRAPHICS_BRUSH_H
#define VELVET_GRAPHICS_BRUSH_H

#include "velvet/support/da.h"
#include "velvet/support/api.h"
#include "velvet/support/result.h"
#include "velvet/support/math.h"
#include "velvet/support/color.h"
#include "velvet/graphics/bitmap.h"

enum vl_graphics_brush_type {
    VL_GRAPHICS_RENDER_BRUSH_SOLID = 1,
    VL_GRAPHICS_RENDER_BRUSH_LINEAR_GRADIENT,
    VL_GRAPHICS_RENDER_BRUSH_BITMAP
};

typedef enum vl_graphics_brush_type vl_graphics_brush_type_t;

enum vl_graphics_brush_extend_mode {
    VL_GRAPHICS_BRUSH_EXTEND_CLAMP = 0,
    VL_GRAPHICS_BRUSH_EXTEND_WRAP,
    VL_GRAPHICS_BRUSH_EXTEND_MIRROR
};

typedef enum vl_graphics_brush_extend_mode vl_graphics_brush_extend_mode_t;

struct vl_graphics_render;
struct vl_graphics_brush {
    vl_graphics_brush_type_t type;
    struct vl_graphics_render *owner;
    vl_graphics_brush_extend_mode_t extend_x, extend_y;
};

typedef struct vl_graphics_brush vl_graphics_brush_t;

struct vl_graphics_brush_solid {
    vl_graphics_brush_t base;
    vl_color_t color;
};

typedef struct vl_graphics_brush_solid vl_graphics_brush_solid_t;

struct vl_graphics_brush_linear_gradient {
    vl_graphics_brush_t base;
    VL_DA(vl_gradient_stop_t) stops;

    // start and end points make up the gradient axis
    vl_point_t start, end; // [0..1]
};

typedef struct vl_graphics_brush_linear_gradient vl_graphics_brush_linear_gradient_t;

enum vl_graphics_brush_bitmap_filter {
    VL_GRAPHICS_BRUSH_BITMAP_FILTER_LINEAR = 0,
    VL_GRAPHICS_BRUSH_BITMAP_FILTER_NEAREST
};

typedef enum vl_graphics_brush_bitmap_filter vl_graphics_brush_bitmap_filter_t;

struct vl_graphics_brush_bitmap {
    vl_graphics_brush_t base;
    vl_graphics_bitmap_t *bitmap;
    vl_graphics_brush_bitmap_filter_t filter;
};

typedef struct vl_graphics_brush_bitmap vl_graphics_brush_bitmap_t;

VL_API vl_graphics_brush_t *vl_graphics_brush_new_solid(struct vl_graphics_render *render, vl_color_t color);
VL_API vl_graphics_brush_t *vl_graphics_brush_new_linear_gradient(struct vl_graphics_render *render, vl_gradient_stop_t *stops, size_t stops_count);
VL_API vl_graphics_brush_t *vl_graphics_brush_new_bitmap(struct vl_graphics_render *render, vl_graphics_bitmap_t *bitmap);
VL_API vl_result_t vl_graphics_brush_free(vl_graphics_brush_t *brush);

#endif // VELVET_GRAPHICS_BRUSH_H