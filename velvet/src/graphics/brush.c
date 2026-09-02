#include "graphics/brush.h"

#include "graphics/bitmap.h"
#include "support/color.h"
#include "graphics/render.h"

#include "support/result.h"

static void setup_brush(vl_graphics_brush_t *brush, vl_graphics_render_t *render) {
    if (brush) {
        brush->owner = render;
    }
}

vl_graphics_brush_t *vl_graphics_brush_new_solid(vl_graphics_render_t *render, vl_color_t color) {
    if (!render || !render->context || !render->context->graphics_brush_new_solid) return NULL;
    vl_graphics_brush_t *brush = render->context->graphics_brush_new_solid(render, color);
    setup_brush(brush, render);
    return brush;
}

vl_graphics_brush_t *vl_graphics_brush_new_linear_gradient(vl_graphics_render_t *render, vl_gradient_stop_t *stops, size_t stops_count) {
    if (!render || !render->context || !render->context->graphics_brush_new_linear_gradient) return NULL;
    vl_graphics_brush_t *brush = render->context->graphics_brush_new_linear_gradient(render, stops, stops_count);
    setup_brush(brush, render);
    return brush;
}

vl_graphics_brush_t *vl_graphics_brush_new_bitmap(vl_graphics_render_t *render, vl_graphics_bitmap_t *bitmap) {
    if (!render || !render->context || !render->context->graphics_brush_new_bitmap) return NULL;
    vl_graphics_brush_t *brush = render->context->graphics_brush_new_bitmap(render, bitmap);
    setup_brush(brush, render);
    return brush;
}

vl_result_t vl_graphics_brush_free(vl_graphics_brush_t *brush) {
    if (!brush || !brush->owner || !brush->owner->context || !brush->owner->context->graphics_brush_free) return VL_ERROR;
    return brush->owner->context->graphics_brush_free(brush);
}