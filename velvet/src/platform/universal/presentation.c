#include "platform/universal/presentation.h"
#include "graphics/presentation.h"
#include "platform/context.h"
#include "platform/universal/window.h"
#include "support/memory.h"
#include "support/result.h"
#include <GLFW/glfw3.h>

/*
    presentation logic for OpenGL is pretty simple as GLFW handles the most of it for us
    in the future when more backends will be added (Vulkan or Metal) presentation logic will be more rich
*/

vl_graphics_presentation_t *vl_graphics_presentation_universal_new(vl_os_window_t *window, vl_graphics_render_t *render) {
    if (!window || !render) return NULL;
    vl_graphics_presentation_universal_t *presentation = VL_NEW(vl_graphics_presentation_universal_t);
    presentation->base.window = window;
    presentation->base.render = render;
    presentation->active = false;
    return (vl_graphics_presentation_t*) presentation;
}

vl_result_t vl_graphics_presentation_universal_begin(vl_graphics_presentation_t *presentation) {
    if (!presentation) return VL_ERROR;
    // nothing to do for us, GLFW handles all of it
    if (((vl_graphics_presentation_universal_t*) presentation)->active) return VL_ERROR;
    ((vl_graphics_presentation_universal_t*) presentation)->active = true;
    return VL_SUCCESS;
}

vl_result_t vl_graphics_presentation_universal_end(vl_graphics_presentation_t *presentation) {
    if (!presentation) return VL_ERROR;
    if (!((vl_graphics_presentation_universal_t*) presentation)->active) return VL_ERROR;
    if (presentation->render->context->types.graphics_render == VL_PLATFORM_CONTEXT_UNIVERSAL) {
        vl_os_window_universal_t *win = (vl_os_window_universal_t*) presentation->window;
        if (!win) return VL_ERROR;
        glfwSwapBuffers(win->handle);
    } else {
        // unsupported render platform context
        return VL_ERROR;
    }
    ((vl_graphics_presentation_universal_t*) presentation)->active = false;
    return VL_SUCCESS;
}

vl_result_t vl_graphics_presentation_universal_free(vl_graphics_presentation_t *presentation) {
    if (!presentation) return VL_ERROR;
    vl_free(presentation);
    return VL_SUCCESS;
}