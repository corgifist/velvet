#include "velvet/graphics/bitmap.h"
#include "velvet/graphics/render.h"

vl_graphics_bitmap_t *vl_graphics_bitmap_new(vl_graphics_render_t *render, 
        size_t width, size_t height, vl_graphics_bitmap_format_t format, void *data) {
    if (!render || !render->context || !render->context->graphics_bitmap_new) return NULL;
    vl_graphics_bitmap_t *bitmap = render->context->graphics_bitmap_new(
        render, width, height, format, data
    );
    if (bitmap) {
        bitmap->owner = render;
    }
    return bitmap;
}

vl_result_t vl_graphics_bitmap_free(vl_graphics_bitmap_t *bitmap) {
    if (!bitmap || !bitmap->owner || !bitmap->owner->context) return VL_ERROR;
    return bitmap->owner->context->graphics_bitmap_free(bitmap);
}