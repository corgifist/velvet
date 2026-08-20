#include "platform/universal/brush.h"
#include "graphics/bitmap.h"
#include "graphics/brush.h"
#include "support/color.h"
#include "graphics/render.h"
#include "platform/universal/render.h"
#include "support/da.h"
#include "support/memory.h"
#include "support/result.h"

vl_graphics_brush_t *vl_graphics_brush_universal_new_solid(vl_graphics_render_t *render, vl_color_t color) {
    if (!render) return NULL;
    void *brush_mem = vl_malloc(sizeof(vl_brush_index_t) + sizeof(vl_graphics_brush_solid_t));
    if (!brush_mem) return NULL;
    vl_brush_index_t *brush_index_ptr = (vl_brush_index_t*) brush_mem;
    vl_graphics_brush_solid_t *brush = VL_PTR_FORWARD(brush_mem, sizeof(vl_brush_index_t));
    brush->base.type = VL_GRAPHICS_RENDER_BRUSH_SOLID;
    brush->base.owner = render;
    brush->color = color;
    *brush_index_ptr = -1;
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) render;
    *VL_DA_PUSH(r->owned_brushes, vl_graphics_brush_t*) = (vl_graphics_brush_t*) brush;
    return (vl_graphics_brush_t*) brush;
}

vl_graphics_brush_t *vl_graphics_brush_universal_new_linear_gradient(vl_graphics_render_t *render, vl_gradient_stop_t *stops, size_t stops_count) {
    if (!render) return NULL;
    void *brush_mem = vl_malloc(sizeof(vl_brush_index_t) + sizeof(vl_graphics_brush_linear_gradient_t));
    if (!brush_mem) return NULL;
    vl_brush_index_t *brush_index_ptr = (vl_brush_index_t*) brush_mem;
    vl_graphics_brush_linear_gradient_t *brush = VL_PTR_FORWARD(brush_mem, sizeof(vl_brush_index_t));
    brush->base.type = VL_GRAPHICS_RENDER_BRUSH_LINEAR_GRADIENT;
    brush->base.owner = render;
    brush->stops = VL_DA_INIT_WITH_CAPACITY(vl_gradient_stop_t, stops_count);
    for (size_t i = 0; i < stops_count; i++) {
        VL_DA_APPEND(brush->stops, stops[i]);
    }
    // default gradient axis (from left to right)
    brush->start = VL_POINT(0, 0.5);
    brush->end = VL_POINT(1, 0.5);
    *brush_index_ptr = -1;
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) render;
    *VL_DA_PUSH(r->owned_brushes, vl_graphics_brush_t*) = (vl_graphics_brush_t*) brush;
    return (vl_graphics_brush_t*) brush;
}

vl_graphics_brush_t *vl_graphics_brush_universal_new_bitmap(vl_graphics_render_t *render, vl_graphics_bitmap_t *bitmap) {
    if (!render || !bitmap) return NULL;
    void *brush_mem = vl_malloc(sizeof(vl_brush_index_t) + sizeof(vl_graphics_brush_bitmap_t));
    if (!brush_mem) return NULL;
    vl_brush_index_t *brush_index_ptr = brush_mem;
    *brush_index_ptr = -1;
    vl_graphics_brush_bitmap_t *brush = VL_PTR_FORWARD(brush_mem, sizeof(vl_brush_index_t));
    brush->bitmap = bitmap;
    brush->base.type = VL_GRAPHICS_RENDER_BRUSH_BITMAP;
    brush->base.owner = render;
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) render;
    *VL_DA_PUSH(r->owned_brushes, vl_graphics_brush_t*) = (vl_graphics_brush_t*) brush;
    return (vl_graphics_brush_t*) brush;
}

vl_result_t vl_graphics_brush_universal_free(vl_graphics_brush_t *brush) {
    if (!brush) return VL_ERROR;
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) brush->owner;
    for (int i = 0; i < VL_DA_LENGTH(r->owned_brushes); i++) {
        if (r->owned_brushes[i] == brush) {
            r->owned_brushes[i] = NULL;
            break;
        }
    }
    switch (brush->type) {
    case VL_GRAPHICS_RENDER_BRUSH_BITMAP:
    case VL_GRAPHICS_RENDER_BRUSH_SOLID: {
        // nothing to free
        break;
    }
    case VL_GRAPHICS_RENDER_BRUSH_LINEAR_GRADIENT: {
        vl_graphics_brush_linear_gradient_t *l = (vl_graphics_brush_linear_gradient_t*) brush;
        VL_DA_FREE(l->stops);
        break;
    }
    }
    vl_free(VL_PTR_BACKWARD(brush, sizeof(vl_brush_index_t)));
    return VL_SUCCESS;
}