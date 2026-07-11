#ifndef VELVET_GRAPHICS_PRESENTATION_H
#define VELVET_GRAPHICS_PRESENTATION_H

#include "velvet/support/result.h"
#include "velvet/os/window.h"
#include "velvet/graphics/render.h"

struct vl_graphics_presentation {
    vl_os_window_t *window;
    vl_graphics_render_t *render;
};

typedef struct vl_graphics_presentation vl_graphics_presentation_t;

vl_graphics_presentation_t *vl_graphics_presentation_new(vl_os_window_t *window, vl_graphics_render_t *render);
vl_result_t vl_graphics_presentation_begin(vl_graphics_presentation_t *presentation);
vl_result_t vl_graphics_presentation_end(vl_graphics_presentation_t *presentation);
vl_result_t vl_graphics_presentation_free(vl_graphics_presentation_t *presentation);

#endif // VELVET_GRAPHICS_PRESENTATION_H