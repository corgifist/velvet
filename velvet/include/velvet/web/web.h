#ifndef VELVET_WEB_H
#define VELVET_WEB_H

#include "velvet/css/layout.h"
#include "velvet/css/stylesheet.h"
#include "velvet/platform/context.h"
#include "velvet/html/document.h"
#include "velvet/dom/render.h"
#include "velvet/graphics/render.h"
#include "velvet/dom/dom.h"
#include "velvet/web/fonts.h"
#include "velvet/web/theme.h"

struct vl_web {
    const char *title;
    vl_dom_t dom;
    const vl_web_theme_t *theme;
    vl_css_stylesheet_t default_stylesheet;
    vl_css_stylesheet_t stylesheet;
    vl_css_layout_node_t root_layout_node;
    bool refresh_styles;

    vl_platform_context_t *platform_context;
    vl_graphics_render_t *render;
    vl_web_fonts_t fonts;
};

typedef struct vl_web vl_web_t;

VL_API vl_result_t vl_web_init(vl_platform_context_t *context, vl_web_t *web, vl_html_document_t *node);
VL_API vl_result_t vl_web_render(vl_web_t *web, vl_dom_render_opts_t *opts);
VL_API vl_result_t vl_web_deinit(vl_web_t *web);

#endif // VELVET_WEB_H