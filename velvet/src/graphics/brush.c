#include "graphics/brush.h"

#include "platform/universal/brush.h"
#include "support/feature.h"
#include "support/memory.h"
#include "support/result.h"

vl_graphics_brush_t *vl_graphics_brush_new_solid(vl_graphics_render_t *render, vl_color_t color) {
    if (!render) return NULL;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_brush_universal_new_solid(render, color);
#endif
    return NULL;
}

vl_result_t vl_graphics_brush_free(vl_graphics_brush_t *brush) {
    if (!brush) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_graphics_brush_universal_free(brush);
#endif
    return VL_ERROR;
}