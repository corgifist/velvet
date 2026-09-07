#ifndef VELVET_DOM_ELEMENT_TEXT_H
#define VELVET_DOM_ELEMENT_TEXT_H

#include "font/atlas.h"
#include "velvet/support/hash.h"
#include "velvet/support/result.h"
#include "velvet/dom/element.h"
#include "velvet/support/memory.h"
#include "velvet/support/da.h"
#include "web/fonts.h"
#include <stdio.h>

struct vl_dom_element_text_blueprint {
    int height;
    int weight;
    VL_DA(VL_DA(vl_web_sized_font_t*)) font_family;
    vl_hash_t font_family_hash;
    vl_hash_t text_hash;
};
typedef struct vl_dom_element_text_blueprint vl_dom_element_text_blueprint_t;

struct vl_dom_element_text_glyph {
    float x, y;
    float advance_x, advance_y;
    int32_t glyph_id, codepoint;
    bool line_break;
    vl_web_font_atlas_codepoint_t web_codepoint;
    vl_font_t *font;
};
typedef struct vl_dom_element_text_glyph vl_dom_element_text_glyph_t;

struct vl_dom_element_text_layout {
    VL_DA(vl_dom_element_text_glyph_t) glyphs;
    vl_dom_element_text_blueprint_t blueprint;
};
typedef struct vl_dom_element_text_layout vl_dom_element_text_layout_t;

struct vl_dom_element_text {
    vl_dom_element_t base;
    VL_DA(char) text;
    vl_dom_element_text_layout_t layout;
};

typedef struct vl_dom_element_text vl_dom_element_text_t;

vl_dom_element_t *vl_dom_element_text_new(const char *tag, vl_source_location_t loc);
vl_result_t vl_dom_element_text_render(vl_dom_element_t *element);
vl_result_t vl_dom_element_text_set_property(vl_dom_element_t *element, const char *property, vl_dom_element_property_type_t type, const void *value);
vl_vec2_t vl_dom_element_text_get_content_size(vl_dom_element_t *element);
vl_result_t vl_dom_element_text_free(vl_dom_element_t *element);

#endif // VELVET_DOM_ELEMENT_TEXT_H