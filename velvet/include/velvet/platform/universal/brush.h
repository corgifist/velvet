#ifndef VELVET_PLATFORM_UNIVERSAL_BRUSH_H
#define VELVET_PLATFORM_UNIVERSAL_BRUSH_H

#include "graphics/bitmap.h"
#include "velvet/support/da.h"
#include "velvet/graphics/color.h"
#include "velvet/support/result.h"
#include "velvet/graphics/brush.h"
#include "velvet/graphics/render.h"

#include <stdint.h>
#if INTPTR_MAX == INT64_MAX
    typedef int64_t vl_brush_index_t;
#else
    typedef int32_t vl_brush_index_t;
#endif

vl_graphics_brush_t *vl_graphics_brush_universal_new_solid(vl_graphics_render_t *render, vl_color_t color);
vl_graphics_brush_t *vl_graphics_brush_universal_new_linear_gradient(vl_graphics_render_t *render, vl_gradient_stop_t *stops, size_t stops_count);
vl_graphics_brush_t *vl_graphics_brush_universal_new_bitmap(vl_graphics_render_t *render, vl_graphics_bitmap_t *bitmap);
vl_result_t vl_graphics_brush_universal_free(vl_graphics_brush_t *brush);

#endif // VELVET_PLATFORM_UNIVERSAL_BRUSH_H