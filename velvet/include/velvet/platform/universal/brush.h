#ifndef VELVET_PLATFORM_UNIVERSAL_BRUSH_H
#define VELVET_PLATFORM_UNIVERSAL_BRUSH_H

#include "velvet/graphics/color.h"
#include "velvet/support/result.h"
#include "velvet/graphics/brush.h"
#include "velvet/graphics/render.h"

struct vl_graphics_brush_universal {
    vl_graphics_brush_t base;
    int brush_index;
};

typedef struct vl_graphics_brush_universal vl_graphics_brush_universal_t;

struct vl_graphics_brush_universal_solid {
    vl_graphics_brush_universal_t base;
    vl_color_t color;
};

typedef struct vl_graphics_brush_universal_solid vl_graphics_brush_universal_solid_t;

vl_graphics_brush_t *vl_graphics_brush_universal_new_solid(vl_graphics_render_t *render, vl_color_t color);
vl_result_t vl_graphics_brush_universal_solid_get_color(vl_graphics_brush_t *brush, vl_color_t *color);
vl_result_t vl_graphics_brush_universal_free(vl_graphics_brush_t *brush);

#endif // VELVET_PLATFORM_UNIVERSAL_BRUSH_H