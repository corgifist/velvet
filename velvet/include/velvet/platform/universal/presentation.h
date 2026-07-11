/*
    platform/universal/presentation.h - presentation primitive for OpenGL 3.3 backend
*/

#ifndef VELVET_PLATFORM_UNIVERSAL_PRESENTATION_H
#define VELVET_PLATFORM_UNIVERSAL_PRESENTATION_H

#include "velvet/graphics/render.h"
#include "velvet/os/window.h"
#include "velvet/support/result.h"
#include "velvet/graphics/presentation.h"

struct vl_graphics_presentation_universal {
    vl_graphics_presentation_t base;
};

typedef struct vl_graphics_presentation_universal vl_graphics_presentation_universal_t;

vl_graphics_presentation_t *vl_graphics_presentation_universal_new(vl_os_window_t *window, vl_graphics_render_t *render);
vl_result_t vl_graphics_presentation_universal_begin(vl_graphics_presentation_t *presentation);
vl_result_t vl_graphics_presentation_universal_end(vl_graphics_presentation_t *presentation);
vl_result_t vl_graphics_presentation_universal_free(vl_graphics_presentation_t *presentation);

#endif // VELVET_PLATFORM_UNIVERSAL_PRESENTATION_H