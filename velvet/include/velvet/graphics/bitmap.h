#ifndef VELVET_GRAPHICS_BITMAP_H
#define VELVET_GRAPHICS_BITMAP_H

#include "velvet/common.h"
#include "velvet/support/api.h"
#include "velvet/support/result.h"

enum vl_graphics_bitmap_format {
    VL_GRAPHICS_BITMAP_FORMAT_RGBA8 = 0
};

typedef enum vl_graphics_bitmap_format vl_graphics_bitmap_format_t;

struct vl_graphics_render;

struct vl_graphics_bitmap {
    struct vl_graphics_render *owner;
    size_t width, height;
    vl_graphics_bitmap_format_t format;
};

typedef struct vl_graphics_bitmap vl_graphics_bitmap_t;

VL_API vl_graphics_bitmap_t *vl_graphics_bitmap_new(struct vl_graphics_render *render, 
    size_t width, size_t height, vl_graphics_bitmap_format_t format, void *data);

VL_API vl_result_t vl_graphics_bitmap_free(vl_graphics_bitmap_t *bitmap);

#endif // VELVET_GRAPHICS_BITMAP_H