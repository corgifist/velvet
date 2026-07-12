#ifndef VELVET_GRAPHICS_GRAPHICS_H
#define VELVET_GRAPHICS_GRAPHICS_H

#include "velvet/support/api.h"
#include "velvet/support/result.h"
#include "velvet/graphics/color.h"
#include "velvet/graphics/geometry.h"
#include "velvet/os/window.h"

struct vl_graphics_render {
    vl_os_window_t *owner;
    const char *backend;
};
typedef struct vl_graphics_render vl_graphics_render_t;

VL_API vl_graphics_render_t *vl_graphics_render_new(vl_os_window_t *window);
VL_API vl_result_t vl_graphics_render_clear(vl_graphics_render_t *render, vl_color_t fill);
VL_API vl_result_t vl_graphics_render_batch_begin(vl_graphics_render_t *render);
VL_API vl_result_t vl_graphics_render_batch_rect(vl_graphics_render_t *render, vl_rect_t rect, vl_color_t fill);
VL_API vl_result_t vl_graphics_render_batch_end(vl_graphics_render_t *render);
VL_API vl_result_t vl_graphics_render_free(vl_graphics_render_t *render);

#endif // VELVET_GRAPHICS_GRAPHICS_H