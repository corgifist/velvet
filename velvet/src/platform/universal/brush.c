#include "platform/universal/brush.h"
#include "graphics/brush.h"
#include "graphics/color.h"
#include "graphics/render.h"
#include "platform/universal/render.h"
#include "support/memory.h"
#include "support/result.h"

vl_graphics_brush_t *vl_graphics_brush_universal_new_solid(vl_graphics_render_t *render, vl_color_t color) {
    if (!render) return NULL;
    vl_graphics_brush_solid_t *brush = vl_malloc(sizeof(int) + sizeof(vl_graphics_brush_solid_t));
    if (!brush) return NULL;
    int *brush_index_ptr = (int*) brush;
    brush = (vl_graphics_brush_solid_t*) (((vl_byte_t*) brush) + sizeof(int));
    brush->base.type = VL_GRAPHICS_RENDER_BRUSH_SOLID;
    brush->base.owner = render;
    brush->color = color;
    *brush_index_ptr = -1;
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) render;
    *VL_DA_PUSH(r->owned_brushes, vl_graphics_brush_t*) = (vl_graphics_brush_t*) brush;
    return (vl_graphics_brush_t*) brush;
}

vl_graphics_brush_t *vl_graphics_brush_universal_new_linear_gradient(vl_graphics_render_t *render, VL_DA(vl_graphics_brush_gradient_stop_t) stops) {
    if (!render) return NULL;
    vl_graphics_brush_linear_gradient_t *brush = vl_malloc(sizeof(int) + sizeof(vl_graphics_brush_linear_gradient_t));
    if (!brush) return NULL;
    int *brush_index_ptr = (int*) brush;
    brush = (vl_graphics_brush_linear_gradient_t*) (((vl_byte_t*) brush) + sizeof(int));
    brush->base.type = VL_GRAPHICS_RENDER_BRUSH_LINEAR_GRADIENT;
    brush->base.owner = render;
    brush->stops = stops;
    *brush_index_ptr = -1;
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
    vl_free(((vl_byte_t*) brush) - sizeof(int));
    return VL_SUCCESS;
}