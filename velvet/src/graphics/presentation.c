#include "graphics/presentation.h"
#include "platform/universal/presentation.h"
#include "support/feature.h"
#include "support/result.h"

vl_graphics_presentation_t *vl_graphics_presentation_new(vl_os_window_t *window, vl_graphics_render_t *render) {
    if (!window || !render) return NULL;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_presentation_universal_new(window, render);
#endif // VL_FEATURE(UNIVERSAL_PLATFORM)
    printf("no implementation for vl_graphics_presentation_new\n");
    return NULL;
}

vl_result_t vl_graphics_presentation_begin(vl_graphics_presentation_t *presentation) {
    if (!presentation) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_presentation_universal_end(presentation);
#endif // VL_FEATURE(UNIVERSAL_PLATFORM)
    printf("no implementation for vl_graphics_presentation_begin\n");
    return VL_ERROR;
}

vl_result_t vl_graphics_presentation_end(vl_graphics_presentation_t *presentation) {
    if (!presentation) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_presentation_universal_begin(presentation);
#endif // VL_FEATURE(UNIVERSAL_PLATFORM)
    printf("no implementation for vl_graphics_presentation_end\n");
    return VL_ERROR;
}

vl_result_t vl_graphics_presentation_free(vl_graphics_presentation_t *presentation) {
    if (!presentation) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_presentation_universal_free(presentation);
#endif // VL_FEATURE(UNIVERSAL_PLATFORM)
    printf("no implementation for vl_graphics_presentation_free\n");
    return VL_ERROR;
}