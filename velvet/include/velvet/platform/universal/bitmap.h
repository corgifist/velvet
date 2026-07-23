#ifndef VELVET_PLATFORM_UNIVERSAL_BITMAP_H
#define VELVET_PLATFORM_UNIVERSAL_BITMAP_H

#include "graphics/render.h"
#include "velvet/graphics/bitmap.h"

#include <glad/gl.h>

struct vl_graphics_bitmap_universal {
    vl_graphics_bitmap_t base;

    GLuint handle;
};

typedef struct vl_graphics_bitmap_universal vl_graphics_bitmap_universal_t;

vl_graphics_bitmap_t *vl_graphics_bitmap_universal_new(vl_graphics_render_t *render, 
    size_t width, size_t height, vl_graphics_bitmap_format_t format, void *data);
vl_result_t vl_graphics_bitmap_universal_free(vl_graphics_bitmap_t *bitmap);

#endif // VELVET_PLATFORM_UNIVERSAL_BITMAP_H