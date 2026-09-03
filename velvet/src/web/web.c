#include "web/web.h"
#include "css/layout.h"
#include "css/stylesheet.h"
#include "dom/dom.h"
#include "dom/element.h"
#include "html/document.h"
#include "platform/context.h"
#include "support/memory.h"
#include "web/fonts.h"
#include "dom/style/style.h"
#include "web/theme.h"

static void propagate_stylesheet(vl_css_layout_node_t *node, vl_css_stylesheet_t *sheet) {
    node->stylesheet = sheet;
    for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
        propagate_stylesheet(node->children[i], sheet);
    }
}

vl_result_t vl_web_init(vl_platform_context_t *context, vl_web_t *web, vl_html_document_t *document) {
    if (!web) return VL_ERROR;
    VL_ZERO_OUT(web);
    web->dom.owner = web;
    vl_dom_init_with_html_node(&web->dom, &document->root);
    web->dom.root->layout.parent = &web->root_layout_node;
    vl_css_stylesheet_init(&web->default_stylesheet, vl_web_theme_default_stylesheet());
    web->theme = vl_web_theme_default();
    vl_css_layout_node_init(&web->root_layout_node, "root");
    propagate_stylesheet(&web->dom.root->layout, &web->stylesheet);

    web->platform_context = context;
    web->title = "velvet";
    web->refresh_styles = true;
    vl_web_fonts_init(&web->fonts, web);
    web->fonts.owner = web;
    vl_web_fonts_add_parts_from_system(&web->fonts, "serif", "Times New Roman");
    vl_web_fonts_add_parts_from_system(&web->fonts, "sans-serif", "Arial");

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
        vl_css_stylesheet_merge(&web->stylesheet, &web->default_stylesheet);
        for (int i = 0; i < VL_DA_LENGTH(stylesheets); i++) {
            vl_css_stylesheet_merge(&web->stylesheet, stylesheets[i]);
        }
        VL_DA_FREE(stylesheets);
        printf("---------------\n");
        vl_css_stylesheet_print(&web->stylesheet);
        printf("---------------\n");
        vl_os_window_t *window = web->render->owner;
        web->root_layout_node.size = VL_VEC2(window->io.window_size.x, window->io.window_size.y);
        vl_dom_element_process(web->dom.root);
        web->refresh_styles = false;
    }
    return vl_dom_render(&web->dom, opts);
}

vl_result_t vl_web_deinit(vl_web_t *web) {
    if (!web) return VL_ERROR;
    vl_css_stylesheet_deinit(&web->default_stylesheet);
    vl_css_stylesheet_deinit(&web->stylesheet);
    vl_css_layout_node_deinit(&web->root_layout_node);
    vl_dom_deinit(&web->dom);
    vl_web_fonts_deinit(&web->fonts);
    return VL_SUCCESS;
}