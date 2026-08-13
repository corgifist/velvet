#include "web/web.h"
#include "css/stylesheet.h"
#include "dom/dom.h"
#include "html/document.h"
#include "platform/context.h"
#include "support/memory.h"
#include "web/fonts.h"
#include "dom/style/style.h"

// default fonts
#include "Roboto/Regular.h"

#include "NotoSans_Arabic/Regular.h"

vl_result_t vl_web_init(vl_platform_context_t *context, vl_web_t *web, vl_html_document_t *document) {
    if (!web) return VL_ERROR;
    memset(web, 0, sizeof(*web));
    vl_dom_init_with_html_node(&web->dom, &document->root);
    web->dom.owner = web;

    web->platform_context = context;
    web->title = "velvet";
    web->refresh_styles = true;
    vl_web_fonts_init(&web->fonts, web);

    web->fonts.owner = web;
    vl_web_fonts_add_font(&web->fonts, "Roboto", Roboto_Regular, VL_ARR_LEN(Roboto_Regular), VL_WEB_FONT_REGULAR);
    vl_web_fonts_add_font(&web->fonts, "Noto Sans Arabic", NotoSansArabic_Regular, VL_ARR_LEN(NotoSansArabic_Regular), VL_WEB_FONT_REGULAR);
    return VL_SUCCESS;
}

static void collect_stylesheets(vl_dom_element_t *element, VL_DA(vl_css_stylesheet_t*) *stylesheets) {
    if (!element) return;
    if (element->tag && strcmp(element->tag, "style") == 0) {
        vl_dom_element_style_t *style = (vl_dom_element_style_t*) element;
        *VL_DA_PUSH(*stylesheets, vl_css_stylesheet_t*) = &style->sheet;
        return;
    }
    if (element->children) {
        for (int i = 0; i < VL_DA_LENGTH(element->children); i++) {
            collect_stylesheets(element->children[i], stylesheets);
        }
    }
}

vl_result_t vl_web_render(vl_web_t *web, vl_dom_render_opts_t *opts) {
    if (!web) return VL_ERROR;
    if (web->refresh_styles) {
        vl_css_stylesheet_deinit(&web->stylesheet);
        vl_css_stylesheet_init_empty(&web->stylesheet);

        VL_DA(vl_css_stylesheet_t*) stylesheets = VL_DA_INIT(vl_css_stylesheet_t*);
        collect_stylesheets(web->dom.root, &stylesheets);
        for (int i = 0; i < VL_DA_LENGTH(stylesheets); i++) {
            vl_css_stylesheet_merge(&web->stylesheet, stylesheets[i]);
        }
        VL_DA_FREE(stylesheets);
        printf("---------------\n");
        vl_css_stylesheet_print(&web->stylesheet);
        printf("---------------\n");
        web->refresh_styles = false;
    }
    return vl_dom_render(&web->dom, opts);
}

vl_result_t vl_web_deinit(vl_web_t *web) {
    if (!web) return VL_ERROR;
    vl_css_stylesheet_deinit(&web->stylesheet);
    vl_dom_deinit(&web->dom);
    vl_web_fonts_deinit(&web->fonts);
    return VL_SUCCESS;
}