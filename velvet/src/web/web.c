#include "web/web.h"
#include "dom/dom.h"
#include "html/document.h"
#include "support/memory.h"
#include "web/fonts.h"

// default fonts
#include "Roboto/Regular.h"

vl_result_t vl_web_init(vl_web_t *web, vl_html_node_t *node) {
    if (!web) return VL_ERROR;
    memset(web, 0, sizeof(*web));
    vl_dom_init_with_html_node(&web->dom, node);
    web->dom.owner = web;
    web->title = "velvet";
    vl_web_fonts_init(&web->fonts);
    web->fonts.owner = web;
    vl_web_fonts_add_font(&web->fonts, "Roboto", Roboto_Regular, VL_ARR_LEN(Roboto_Regular), VL_WEB_FONT_REGULAR);
    return VL_SUCCESS;
}

vl_result_t vl_web_render(vl_web_t *web, vl_dom_render_opts_t *opts) {
    if (!web) return VL_ERROR;
    return vl_dom_render(&web->dom, opts);
}

vl_result_t vl_web_deinit(vl_web_t *web) {
    if (!web) return VL_ERROR;
    return VL_SUCCESS;
}