#ifndef VELVET_DOM_BEHAVIOR_TEXT_H
#define VELVET_DOM_BEHAVIOR_TEXT_H

#include "dom/element.h"
#include "velvet/support/da.h"
#include "velvet/web/fonts.h"

struct vl_dom_behavior_text_glyph {
    uint32_t id;
    float x, y;
    float advance_x, advance_y;
    bool newline;
    vl_font_t *font;
};

typedef struct vl_dom_behavior_text_glyph vl_dom_behavior_text_glyph_t;
typedef VL_DA(vl_dom_behavior_text_glyph_t) vl_dom_behavior_text_glyphs_t;

struct vl_dom_behavior_text_font_blueprint {
    int height;
    vl_web_font_weight_t weight;
};

typedef struct vl_dom_behavior_text_font_blueprint vl_dom_behavior_text_font_blueprint_t;

struct vl_dom_behavior_text_layout {
    vl_dom_behavior_text_glyphs_t glyphs;
    const char *priority_font_family;
    vl_dom_behavior_text_font_blueprint_t font_blueprint;
};

typedef struct vl_dom_behavior_text_layout vl_dom_behavior_text_layout_t;

vl_result_t vl_dom_behavior_text_layout_new(vl_dom_element_t *element, 
    vl_dom_behavior_text_layout_t *layout, const char *text);
vl_result_t vl_dom_behavior_text_layout_free(vl_dom_element_t *element, vl_dom_behavior_text_layout_t *layout);

#endif // VELVET_DOM_BEHAVIOR_TEXT_H