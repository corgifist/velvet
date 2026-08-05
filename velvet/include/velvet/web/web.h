#ifndef VELVET_WEB_H
#define VELVET_WEB_H

#include "velvet/graphics/render.h"
#include "velvet/platform/context.h"
#include "velvet/font/font.h"
#include "velvet/font/shaper.h"
#include "velvet/dom/dom.h"

struct vl_web {
    const char *title;
    vl_dom_t dom;

    vl_platform_context_t *platform_context;
    vl_graphics_render_t *render;
    vl_font_t *font;
    vl_font_shaper_t *shaper;
};

typedef struct vl_web vl_web_t;

VL_API vl_result_t vl_web_init(vl_web_t *web, vl_dom_t dom);
VL_API vl_result_t vl_web_deinit(vl_web_t *web);

#endif // VELVET_WEB_H