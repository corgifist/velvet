#include "platform/universal/brush.h"
#include "graphics/brush.h"
#include "graphics/color.h"
#include "graphics/render.h"
#include "support/memory.h"
#include "support/result.h"

vl_graphics_brush_t *vl_graphics_brush_universal_new_solid(vl_graphics_render_t *render, vl_color_t color) {
    if (!render) return NULL;
    vl_graphics_brush_universal_solid_t *brush = VL_NEW(vl_graphics_brush_universal_solid_t);
    if (!brush) return NULL;
    brush->base.type = VL_GRAPHICS_RENDER_BRUSH_SOLID;
    brush->base.owner = render;
    brush->color = color;
    return (vl_graphics_brush_t*) brush;
}

vl_result_t vl_graphics_brush_universal_free(vl_graphics_brush_t *brush) {
    if (!brush) return VL_ERROR;
    vl_free(brush);
    return VL_SUCCESS;
}