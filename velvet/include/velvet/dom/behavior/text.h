#ifndef VELVET_DOM_BEHAVIOR_TEXT_H
#define VELVET_DOM_BEHAVIOR_TEXT_H

#include "dom/element.h"
#include "dom/render.h"
#include "velvet/support/da.h"
#include "velvet/web/fonts.h"

struct vl_dom_behavior_text_glyph {
    uint32_t id, codepoint;
    float x, y;
    float advance_x, advance_y;
    bool newline;
    vl_font_t *font;
};

typedef struct vl_dom_behavior_text_glyph vl_dom_behavior_text_glyph_t;
typedef VL_DA(vl_dom_behavior_text_glyph_t) vl_dom_behavior_text_glyphs_t;

struct vl_dom_behavior_text_layout {
    vl_dom_behavior_text_glyphs_t glyphs;
    const char *priority_font_family;
};

typedef struct vl_dom_behavior_text_layout vl_dom_behavior_text_layout_t;

vl_result_t vl_dom_behavior_text_layout_new(vl_dom_element_t *element, 
    vl_dom_behavior_text_layout_t *layout, const char *text);
vl_result_t vl_dom_behavior_text_layout_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts, 
    vl_dom_behavior_text_layout_t *layout);
vl_vec2_t vl_dom_behavior_text_layout_get_size(vl_dom_element_t *element, vl_dom_behavior_text_layout_t *layout);
vl_result_t vl_dom_behavior_text_layout_free(vl_dom_element_t *element, vl_dom_behavior_text_layout_t *layout);

#endif // VELVET_DOM_BEHAVIOR_TEXT_H