#include "platform/universal/brush.h"
#include "graphics/brush.h"
#include "graphics/color.h"
#include "graphics/render.h"
#include "platform/universal/render.h"
#include "support/memory.h"
#include "support/result.h"

vl_graphics_brush_t *vl_graphics_brush_universal_new_solid(vl_graphics_render_t *render, vl_color_t color) {
    if (!render) return NULL;
    vl_graphics_brush_universal_solid_t *brush = VL_NEW(vl_graphics_brush_universal_solid_t);
    if (!brush) return NULL;
    brush->base.base.type = VL_GRAPHICS_RENDER_BRUSH_SOLID;
    brush->base.base.owner = render;
    brush->base.brush_index = -1;
    brush->color = color;
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) render;
    *VL_DA_PUSH(r->owned_brushes, vl_graphics_brush_t*) = (vl_graphics_brush_t*) brush;
    return (vl_graphics_brush_t*) brush;
}

vl_result_t vl_graphics_brush_universal_solid_get_color(vl_graphics_brush_t *brush, vl_color_t *color) {
    if (!brush || !color) return VL_ERROR;
    if (brush->type != VL_GRAPHICS_RENDER_BRUSH_SOLID) return VL_ERROR;
    vl_graphics_brush_universal_solid_t *solid = (vl_graphics_brush_universal_solid_t*) brush;
    *color = solid->color;
    return VL_SUCCESS;
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
    vl_free(brush);
    return VL_SUCCESS;
}