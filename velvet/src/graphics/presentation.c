#include "graphics/presentation.h"
#include "platform/context.h"
#include "platform/universal/presentation.h"
#include "support/feature.h"
#include "support/result.h"

static bool presentation_valid(vl_graphics_presentation_t *presentation) {
    if (!presentation || !presentation->render->context || !presentation->window->context) return false;
    if (presentation->render->context != presentation->window->context) return false;
    if (!vl_platform_context_valid(presentation->render->context)) return false;
    return true;
}

vl_graphics_presentation_t *vl_graphics_presentation_new(vl_os_window_t *window, vl_graphics_render_t *render) {
    if (!window || !render || !window->context || !render->context) return NULL;
    if (window->context != render->context) return NULL;
    vl_graphics_presentation_t *presentation = render->context->graphics_presentation_new(window, render);
    if (presentation) {
        presentation->window = window;
        presentation->render = render;
    }
    return presentation;
}

vl_result_t vl_graphics_presentation_begin(vl_graphics_presentation_t *presentation) {
    if (!presentation || !presentation_valid(presentation)) return VL_ERROR;
    return presentation->render->context->graphics_presentation_begin(presentation);
}

vl_result_t vl_graphics_presentation_end(vl_graphics_presentation_t *presentation) {
    if (!presentation || !presentation_valid(presentation)) return VL_ERROR;
    return presentation->render->context->graphics_presentation_end(presentation);
}

vl_result_t vl_graphics_presentation_free(vl_graphics_presentation_t *presentation) {
    if (!presentation || !presentation_valid(presentation)) return VL_ERROR;
    return presentation->render->context->graphics_presentation_free(presentation);
}