#ifndef VELVET_GRAPHICS_BRUSH_H
#define VELVET_GRAPHICS_BRUSH_H

#include "velvet/common.h"
#include "velvet/support/da.h"
#include "velvet/support/api.h"
#include "velvet/support/result.h"
#include "velvet/graphics/color.h"

enum vl_graphics_brush_type {
    VL_GRAPHICS_RENDER_BRUSH_SOLID = 1,
    VL_GRAPHICS_RENDER_BRUSH_LINEAR_GRADIENT
};

typedef enum vl_graphics_brush_type vl_graphics_brush_type_t;

struct vl_graphics_render;

struct vl_graphics_brush {
    vl_graphics_brush_type_t type;
    struct vl_graphics_render *owner;
};

typedef struct vl_graphics_brush vl_graphics_brush_t;

struct vl_graphics_brush_solid {
    vl_graphics_brush_t base;
    vl_color_t color;
};

typedef struct vl_graphics_brush_solid vl_graphics_brush_solid_t;

struct vl_graphics_brush_gradient_stop {
    float percentage;
    vl_color_t color;
};

typedef struct vl_graphics_brush_gradient_stop vl_graphics_brush_gradient_stop_t;

struct vl_graphics_brush_linear_gradient {
    vl_graphics_brush_t base;
    VL_DA(vl_graphics_brush_gradient_stop_t) stops;
};

typedef struct vl_graphics_brush_linear_gradient vl_graphics_brush_linear_gradient_t;

VL_API vl_graphics_brush_t *vl_graphics_brush_new_solid(struct vl_graphics_render *render, vl_color_t color);
VL_API vl_graphics_brush_t *vl_graphics_brush_new_linear_gradient(struct vl_graphics_render *render, VL_DA(vl_graphics_brush_gradient_stop_t) stops);
VL_API vl_result_t vl_graphics_brush_free(vl_graphics_brush_t *brush);

#endif // VELVET_GRAPHICS_BRUSH_H